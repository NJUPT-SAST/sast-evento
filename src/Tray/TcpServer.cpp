#include "TcpServer.h"
#include <QApplication>

TcpServer::TcpServer()
    : server(new QTcpServer)
    , socket(nullptr) {
    server->moveToThread(&thread);
    QObject::connect(server, &QTcpServer::newConnection, this, &TcpServer::readData);
    QObject::connect(&thread, &QThread::started, [this]() {
        qDebug() << "Listening on port 1920";
        if (!server->listen(QHostAddress::Any, 1920)) {
            qDebug() << "Failed to listen on port 1920";
        }
    });
    QObject::connect(&thread, &QThread::finished, server, &QObject::deleteLater);

    thread.start();
}

void TcpServer::readData() {
    socket = server->nextPendingConnection();
    QObject::connect(socket, &QTcpSocket::readyRead, [this] {
        auto data = socket->readAll();
        if (data == Message::EXIT) {
            emit exitAppReceived();
        } else {
            emit showMessageReceived(std::move(data));
        }
    });
}

void TcpServer::sendShowWindow() {
    sendData(Message::SHOW);
}

void TcpServer::sendShowAboutPage() {
    sendData(Message::ABOUT);
}

void TcpServer::sendExitApp() {
    sendData(Message::EXIT);
    QApplication::exit();
}

void TcpServer::sendData(QByteArray const& data) {
    if (!socket) {
        return;
    }
    socket->write(data);
    socket->flush();
    socket->waitForBytesWritten();
}

TcpServer::~TcpServer() {
    thread.quit();
    thread.wait();
}