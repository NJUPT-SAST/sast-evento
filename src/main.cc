#include <Controller/UiBridge.h>
#include <Infrastructure/IPC/SocketClient.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Logger.hh>
#include <Version.h>
#include <filesystem>
#include <spdlog/spdlog.h>
#ifdef PLATFORM_LINUX
#include <libintl.h>
#include <locale>
#endif

#include "Infrastructure/System/Crash.h"
#include "Infrastructure/System/Signal.h"

void signalCallback(int signal) {
    // Here we can save crash log
    std::string msg = "Signal " + std::to_string(signal) + " received.";
    saveCrashLog(msg);
}

int main(int argc, char** argv) {
    // Here we register signal handler
    SignalHandler::getInstance().registerHandler(SIGABRT, signalCallback);
    SignalHandler::getInstance().registerHandler(SIGSEGV, signalCallback);
    Logger logger(
#ifdef EVENTO_DEBUG
        Logger::Level::debug,
#else
        Logger::Level::info,
#endif
        (std::filesystem::temp_directory_path() / "NJUPT-SAST" / "logs" / "evento.log").string());
    evento::initConfig();
    spdlog::info("SAST Evento version: v" VERSION_FULL);

#ifdef PLATFORM_LINUX
    bindtextdomain("sast-evento", evento::localePath.string().c_str());
    std::locale::global(std::locale(""));
    spdlog::info("locale: {}", std::locale::global(std::locale("")).name());
#endif

    evento::UiBridge uiBridge(App::create());

    evento::SocketClient socketClient({
        {evento::SocketClient::MessageType::ShowWindow, [&uiBridge] { uiBridge.show(); }},
        {evento::SocketClient::MessageType::ShowAboutPage,
         [&uiBridge] {
             uiBridge.show();
             uiBridge.getViewManager().navigateTo(ViewName::AboutPage);
         }},
        {evento::SocketClient::MessageType::ExitApp, [&uiBridge] { uiBridge.exit(); }},
    });

    socketClient.startTray();

    // block until exit event loop
    uiBridge.run();

    socketClient.exitTray();

    evento::saveConfig();
}