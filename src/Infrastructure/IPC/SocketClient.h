#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace evento {

namespace net = boost::asio;

class SocketClient {
public:
    SocketClient(net::io_context& io_context,
                 std::unordered_map<std::string_view, std::function<void()>> actions);
    ~SocketClient();

    net::awaitable<void> connect(std::uint16_t port);
    net::awaitable<void> send(std::string const& message);
    net::awaitable<std::string> receive();
    void close();

    struct MessageType {
        static constexpr std::string_view ShowWindow = "SHOW";
        static constexpr std::string_view ShowAboutPage = "ABOUT";
        static constexpr std::string_view ExitApp = "EXIT";
    };

private:
    void handleReceive(std::string const& message);

    inline static SocketClient* _instance = nullptr;

    net::io_context& _ioc;
    std::unique_ptr<net::ip::tcp::socket> _socket;
    std::unordered_map<std::string_view, std::function<void()>> _actions;

    friend SocketClient* ipc();
};

SocketClient* ipc();

} // namespace evento