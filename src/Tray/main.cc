#include "TcpServer.h"
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qsystemtrayicon.h>

void exitApp() {
    QApplication::exit();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    TcpServer server;

    auto* tray = new QSystemTrayIcon(QIcon(":/img/evento.png"));
    tray->setToolTip(QStringLiteral("SAST Evento"));
    auto* menu = new QMenu();
    QAction* showAction = menu->addAction("显示");
    QObject::connect(showAction, &QAction::triggered, &server, &TcpServer::sendShowWindow);
    QAction* aboutAction = menu->addAction("关于");
    QObject::connect(aboutAction, &QAction::triggered, &server, &TcpServer::sendShowAboutPage);
    QAction* closeAction = menu->addAction("退出");
    QObject::connect(closeAction, &QAction::triggered, &server, &TcpServer::sendExitApp);
    QObject::connect(tray, &QSystemTrayIcon::messageClicked, &server, &TcpServer::sendShowWindow);
    QObject::connect(tray,
                     &QSystemTrayIcon::activated,
                     [&server](QSystemTrayIcon::ActivationReason reason) {
                         if (reason == QSystemTrayIcon::Trigger) {
                             server.sendShowWindow();
                         }
                     });
    tray->setContextMenu(menu);

    QObject::connect(&server, &TcpServer::exitAppReceived, exitApp);
    QObject::connect(&server, &TcpServer::showMessageReceived, [tray](QByteArray data) {
        tray->showMessage("SAST Evento", data, QIcon(":/img/evento.png"));
    });

    tray->show();

    return QApplication::exec();
}
