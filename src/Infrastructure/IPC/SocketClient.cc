#include "SocketClient.h"
#include <Controller/AsyncExecutor.hh>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <exception>
#include <spdlog/spdlog.h>
#include <utility>

namespace evento {

SocketClient::SocketClient(net::io_context& io_context,
                           std::unordered_map<std::string_view, std::function<void()>> actions)
    : _ioc(io_context)
    , _actions(std::move(actions)) {
    if (!_instance)
        _instance = this;
    evento::executor()->asyncExecute(evento::ipc()->connect(1920), []() {});
}

SocketClient::~SocketClient() {
    close();
}

net::awaitable<void> SocketClient::connect(std::uint16_t port) {
    _socket = std::make_unique<net::ip::tcp::socket>(_ioc);
    net::ip::tcp::endpoint endpoint(net::ip::make_address("127.0.0.1"), port);
    co_await _socket->async_connect(endpoint, net::use_awaitable);
    net::co_spawn(
        _ioc,
        [this] { return receive(); },
        [this](std::exception_ptr e, std::string message) {
            if (e) {
                try {
                    std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    spdlog::error("Error: {}", ex.what());
                    return;
                }
            }

            handleReceive(message);
        });
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

net::awaitable<void> SocketClient::send(std::string const& message) {
    if (!_socket) {
        spdlog::error("Socket is not connected");
        co_return;
    }

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

void SocketClient::handleReceive(std::string const& message) {
    spdlog::info("Received: {}", message);
    auto action = _actions.find(message);
    if (action != _actions.end()) {
        action->second();
    } else {
        spdlog::warn("Unknown message: {}", message);
    }
}

SocketClient* ipc() {
    return SocketClient::_instance;
}

} // namespace evento