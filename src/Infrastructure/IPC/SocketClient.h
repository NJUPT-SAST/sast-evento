#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/process.hpp>
#include <chrono>
#include <functional>
#include <unordered_map>

namespace evento {

namespace net = boost::asio;

class SocketClient {
public:
    SocketClient(std::unordered_map<std::string_view, std::function<void()>> actions);
    ~SocketClient();

    void startTray();
    void exitTray();

    void showOrUpdateMessage(int messageId,
                             std::string const& message,
                             std::chrono::system_clock::time_point const& time);

    void cancelMessage(int messageId);

    struct MessageType {
        static constexpr std::string_view ShowWindow = "SHOW";
        static constexpr std::string_view ShowAboutPage = "ABOUT";
        static constexpr std::string_view ExitApp = "EXIT";
    };

    inline static SocketClient* _instance = nullptr;

private:
    boost::process::child _tray;

    net::awaitable<void> handleReceive(std::string const& message);

    net::awaitable<void> connect(std::uint16_t port);
    net::awaitable<void> send(std::string message);
    net::awaitable<std::string> receive();
    void close();

    std::unique_ptr<net::ip::tcp::socket> _socket;
    std::unordered_map<std::string_view, std::function<void()>> _actions;

    std::unordered_map<int, std::string> _messageMap;
};

SocketClient* ipc();

} // namespace evento