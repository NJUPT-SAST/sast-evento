#pragma once

#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QObject {
    Q_OBJECT
public:
    TcpServer();
    ~TcpServer();

    struct Message {
        inline static const QByteArray SHOW = "SHOW";
        inline static const QByteArray ABOUT = "ABOUT";
        inline static const QByteArray EXIT = "EXIT";
    };

signals:
    void showMessageReceived(QByteArray data);
    void exitAppReceived();

public slots:
    void sendShowWindow();
    void sendShowAboutPage();
    void sendExitApp();

private slots:
    void readData();
    void sendData(QByteArray const& data);

private:
    QTcpServer* server{};
    QTcpSocket* socket{};
};