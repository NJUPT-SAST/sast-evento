#include <Infrastructure/Network/network.h>
#include <Infrastructure/Utils/Logger.h>
#include <app.h>

int main(int argc, char** argv) {
    INIT_LOGGER(Logger::Level::debug, "logs/evento.log", 8192, 1);
    auto ui = App::create();
    ui->on_loginClicked(start_sast_link);
    ui->run();
}
