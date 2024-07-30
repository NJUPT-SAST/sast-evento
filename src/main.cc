#include <Infrastructure/Network/network.h>
#include <Infrastructure/Utils/Logger.h>
#include <app.h>

int main(int argc, char** argv) {
    Logger logger;
    logger.initLogger(Logger::Level::debug, "logs/evento.log", 8192, 1);
    spdlog::info("SAST Evento version: v" VERSION_FULL);
    auto ui = App::create();
    ui->on_loginClicked(start_sast_link);
    ui->run();
}
