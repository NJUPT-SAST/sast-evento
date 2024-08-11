#include <Controller/UiBridge.h>
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
    uiBridge.run();
    evento::saveConfig();
}