#include <Infrastructure/Network/network.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Logger.h>
#include <Version.h>
#include <app.h>
#include <filesystem>
#include <slint.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    Logger logger(Logger::Level::debug,
                  (std::filesystem::temp_directory_path() / "NJUPT-SAST" / "logs" / "evento.log")
                      .string());
    evento::initConfig();
    spdlog::info("SAST Evento version: v" VERSION_FULL);
    auto uiEntry = App::create();
    uiEntry->global<LoginOverlayBridge>().on_link_login(start_sast_link);
    uiEntry->global<LoginOverlayBridge>().set_version("v" VERSION_FULL);
    uiEntry->run();
    evento::saveConfig();
}