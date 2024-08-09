#include <Controller/UiBridge.h>
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

    evento::UiBridge uiBridge(App::create());
    uiBridge.run();
    evento::saveConfig();
}