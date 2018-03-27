#pragma once
#include "ui_client.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QWidget>

namespace Ui
{
    class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = Q_NULLPTR);
    ~Client();
    void readData();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    Ui::Client *ui;
    QTcpSocket *tcpSocket = nullptr;
    QTextStream in;
};
