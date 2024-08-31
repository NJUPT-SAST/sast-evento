#include "slint.h"
#include <Controller/UiBridge.h>
#include <Infrastructure/IPC/SocketClient.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Logger.hh>
#include <Version.h>
#include <filesystem>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    Logger logger(Logger::Level::debug,
                  (std::filesystem::temp_directory_path() / "NJUPT-SAST" / "logs" / "evento.log")
                      .string());
    evento::initConfig();
    spdlog::info("SAST Evento version: v" VERSION_FULL);

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

    uiBridge.run(evento::settings.minimalToTray ? slint::EventLoopMode::RunUntilQuit
                                                : slint::EventLoopMode::QuitOnLastWindowClosed);

    evento::saveConfig();
}