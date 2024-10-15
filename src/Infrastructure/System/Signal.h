#pragma once

#include <csignal>
#include <functional>
#include <string>
#include <unordered_map>

class SignalHandler {
public:
    using Callback = std::function<void(int)>;

    static SignalHandler& getInstance();

    void registerHandler(int signal, Callback callback);

    static std::string getStackTrace();

private:
    SignalHandler();
    std::unordered_map<int, Callback> handlers;

    static void handleSignal(int signal);
};
