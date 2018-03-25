#pragma once
#include "ui_server.h"
#include <QtNetwork>
#include <QWidget>

namespace Ui
{
    class Server;
}

class Server : public QWidget
{
    Q_OBJECT
public:
    Server(QWidget *parent = Q_NULLPTR);
    ~Server();
    addClient();

private:
    Ui::Server *ui;
    QTcpServer *tcpServer;
};
