#include "SocketClient.h"
#include <Controller/AsyncExecutor.hh>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <boost/process.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <slint.h>
#include <spdlog/spdlog.h>

namespace evento {

namespace bp = boost::process;

SocketClient::SocketClient(std::unordered_map<std::string_view, std::function<void()>> actions)
    : _actions(std::move(actions)) {
    if (!_instance)
        _instance = this;
}

SocketClient::~SocketClient() {
    close();
}

void SocketClient::startTray() {
    bp::ipstream pipe;
    boost::filesystem::path trayPath = boost::filesystem::current_path().parent_path() / "Tray";
#if defined(EVENTO_DEBUG)
#ifdef _WIN32
    trayPath /= "Debug/sast-evento-tray.exe";
#else
    trayPath /= "Debug/sast-evento-tray";
#endif
#else
#ifdef _WIN32
    trayPath /= "Release/sast-evento-tray.exe";
#else
    trayPath /= "Release/sast-evento-tray";
#endif
#endif
    bp::child tray(trayPath, bp::std_out > pipe, bp::std_err > bp::null);

    std::string line;
    if (!pipe || !std::getline(pipe, line) || line.empty()) {
        return;
    }

    spdlog::info("Tray started at: {}", line);

    net::co_spawn(executor()->getIoContext(),
                  connect(std::strtol(line.c_str(), nullptr, 10)),
                  net::detached);

    tray.detach();
}

void SocketClient::exitTray() {
    net::co_spawn(_socket->get_executor(), send("EXIT"), net::detached);
}

void SocketClient::showOrUpdateMessage(int messageId,
                                       std::string const& message,
                                       std::chrono::steady_clock::time_point const& time) {
    if (messageId == 0) {
        spdlog::warn("Invalid message id");
        return;
    }
    if (_messageMap.contains(messageId)) {
        _messageMap.erase(messageId);
    }
    _messageMap[messageId] = message;
    evento::executor()->asyncExecute(
        [messageId, this]() -> net::awaitable<void> {
            if (_messageMap.contains(messageId)) {
                co_await ipc()->send(_messageMap[messageId]);
                _messageMap.erase(messageId);
            }
        },
        []() {},
        time - std::chrono::steady_clock::now(),
        AsyncExecutor::Delay | AsyncExecutor::Once);
}

void SocketClient::cancelMessage(int messageId) {
    if (messageId == 0) {
        spdlog::warn("Invalid message id");
        return;
    }
    if (_messageMap.contains(messageId)) {
        _messageMap.erase(messageId);
    }
}

net::awaitable<void> SocketClient::connect(std::uint16_t port) {
    _socket = std::make_unique<net::ip::tcp::socket>(co_await net::this_coro::executor);
    net::ip::tcp::endpoint endpoint(net::ip::make_address("127.0.0.1"), port);

    co_await _socket->async_connect(endpoint, net::use_awaitable);

    for (;;) {
        net::co_spawn(_socket->get_executor(), handleReceive(co_await receive()), net::detached);
    }
}

net::awaitable<std::string> SocketClient::receive() {
    if (!_socket) {
        spdlog::error("Socket is not connected");
        co_return "";
    }

    std::string data;
    data.resize(1024);
    auto size = co_await _socket->async_receive(net::buffer(data), 0, net::use_awaitable);
    data.resize(size);
    co_return data;
}

net::awaitable<void> SocketClient::send(std::string message) {
    if (!_socket) {
        spdlog::error("Socket is not connected");
        co_return;
    }

    spdlog::info("IPC Send: {}", message);
    co_await net::async_write(*_socket, net::buffer(message), net::use_awaitable);
}

void SocketClient::close() {
    if (_socket) {
        boost::system::error_code ec;
        ec = _socket->shutdown(net::socket_base::shutdown_both, ec);
        ec = _socket->close(ec);
        _socket.reset();
    }
}

net::awaitable<void> SocketClient::handleReceive(std::string const& message) {
    spdlog::info("IPC Received: {}", message);
    auto action = _actions.find(message);
    if (action != _actions.end()) {
        slint::invoke_from_event_loop(action->second);
    } else {
        spdlog::warn("Unknown message: {}", message);
    }
    co_return;
}

SocketClient* ipc() {
    return SocketClient::_instance;
}

} // namespace evento