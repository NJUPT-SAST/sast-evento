#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <qapplication.h>

void showMainWindow() {}

void showAboutPage() {}

void exitApp() {
    QApplication::exit();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto* tray = new QSystemTrayIcon(QIcon(":/img/evento.svg"));
    tray->setToolTip(QStringLiteral("SAST Evento"));
    auto* menu = new QMenu();
    QAction* showAction = menu->addAction("显示");
    QObject::connect(showAction, &QAction::triggered, showMainWindow);
    QAction* aboutAction = menu->addAction("关于");
    QObject::connect(aboutAction, &QAction::triggered, showAboutPage);
    QAction* closeAction = menu->addAction("退出");
    QObject::connect(closeAction, &QAction::triggered, exitApp);
    tray->setContextMenu(menu);
    tray->show();

    return QApplication::exec();
}
