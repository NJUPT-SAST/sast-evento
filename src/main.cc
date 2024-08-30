#include <Controller/UiBridge.h>
#include <Infrastructure/IPC/SocketClient.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Logger.hh>
#include <Version.h>
#include <boost/process.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <thread>

int main(int argc, char** argv) {
    Logger logger(Logger::Level::debug,
                  (std::filesystem::temp_directory_path() / "NJUPT-SAST" / "logs" / "evento.log")
                      .string());
    evento::initConfig();
    spdlog::info("SAST Evento version: v" VERSION_FULL);

    evento::UiBridge uiBridge(App::create());

    boost::filesystem::path trayPath = boost::filesystem::current_path().parent_path() / "Tray";
#if defined(EVENTO_DEBUG)
    trayPath /= "Debug/sast-evento-tray";
#else
    trayPath /= "Release/sast-evento-tray";
#endif

    boost::process::child tray(trayPath);

    evento::SocketClient socketClient(evento::executor()->getIoContext(),
                                      {
                                          {evento::SocketClient::MessageType::ShowWindow,
                                           [&uiBridge] { uiBridge.show(); }},
                                          {evento::SocketClient::MessageType::ShowAboutPage,
                                           [&uiBridge] {
                                               uiBridge.show();
                                               uiBridge.getViewManager().navigateTo(
                                                   ViewName::AboutPage);
                                           }},
                                          {evento::SocketClient::MessageType::ExitApp,
                                           [&uiBridge] { uiBridge.exit(); }},
                                      });

    uiBridge.run();

    if (tray.joinable())
        tray.join();

    evento::saveConfig();
}