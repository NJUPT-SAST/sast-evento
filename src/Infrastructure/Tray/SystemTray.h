#pragma once

#include <app.h>
#include <slint.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <tray.hpp>

namespace evento {

class SystemTray {
public:
    SystemTray(slint::ComponentWeakHandle<App> uiWeak)
        : _tray("Evento Tray", "./app.ico") {
        std::thread trayThread([&] {
            _tray.addEntry(Tray::Button("显示", [&] {
                auto ui = *uiWeak.lock();
                ui->show();
            }));
            _tray.addEntry(Tray::Button("关于", [&] {
                //TODO: switch to about page
            }));
            _tray.addEntry(Tray::Button("退出", [&] {
                spdlog::info("exiting SAST Evento...");
                _tray.exit();
                slint::quit_event_loop();
                _isRunning = false;
            }));
            _tray.run();
        });
        trayThread.detach();
    }
    ~SystemTray() {
        if (_isRunning) {
            _tray.exit();
        }
    }

private:
    Tray::Tray _tray;
    bool _isRunning = true;
};

} // namespace evento