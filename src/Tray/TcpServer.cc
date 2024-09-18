#include "TcpServer.h"
#include <QApplication>
#include <iostream>

TcpServer::TcpServer()
    : server(new QTcpServer)
    , socket(nullptr) {
    QObject::connect(server, &QTcpServer::newConnection, this, &TcpServer::readData);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        std::cerr << "Failed to listen on port" << server->serverPort() << std::endl;
    } else {
        std::cout << server->serverPort() << std::endl;
    }
}

void TcpServer::readData() {
    socket = server->nextPendingConnection();
    QObject::connect(socket, &QTcpSocket::readyRead, [this] {
        auto data = socket->readAll();
        qDebug() << data;
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
    if (socket) {
        socket->close();
        socket->deleteLater();
    }
}