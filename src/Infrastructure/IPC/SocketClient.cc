#include <Controller/AsyncExecutor.hh>
#include <Infrastructure/IPC/SocketClient.h>
#include <boost/asio.hpp>
#include <boost/dll.hpp>
#include <boost/system.hpp>
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
    boost::filesystem::path trayPath = boost::dll::program_location().parent_path();
#ifdef EVENTO_DEBUG
    trayPath /= "../Tray/Debug";
#endif
#ifdef PLATFORM_WINDOWS
    trayPath /= "sast-evento-tray.exe";
#else
    trayPath /= "sast-evento-tray";
#endif

    std::error_code ec;
    bp::child tray(trayPath, bp::std_in<bp::close, bp::std_out> pipe, bp::std_err > bp::null, ec);
    if (ec) {
        spdlog::error("Failed to start tray: file = {}, reason = {}",
                      trayPath.string(),
                      ec.message());
        return;
    }

    std::string line;
    if (!pipe || !std::getline(pipe, line) || line.empty()) {
        spdlog::error("Failed to start tray: file = {}, reason = {}", trayPath.string(), "no port");
        return;
    }
    _tray = std::move(tray);

    spdlog::info("Tray started at: {}", line);

    net::co_spawn(executor()->getIoContext(),
                  connect(std::strtol(line.c_str(), nullptr, 10)),
                  net::detached);
}

void SocketClient::exitTray() {
    std::error_code ec;
    if (_socket) {
        net::co_spawn(_socket->get_executor(), send("EXIT"), net::detached);
        _tray.wait(ec);
    } else {
        _tray.terminate(ec);
    }
    auto exit_code = _tray.exit_code();
    spdlog::info("Tray exited with code: {}", exit_code);
}

void SocketClient::showOrUpdateMessage(int messageId,
                                       std::string const& message,
                                       std::chrono::system_clock::time_point const& time) {
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
            // A factory function to create awaitable task when the timer is triggered
            if (_messageMap.contains(messageId)) {
                auto message = std::move(_messageMap[messageId]);
                _messageMap.erase(messageId);
                return this->send(std::move(message));
            } else {
                // do nothing
                return []() -> net::awaitable<void> { co_return; }();
            }
        },
        []() {},
        time - std::chrono::system_clock::now(),
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

void SocketClient::deleteAllMessage() {
    _messageMap.clear();
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