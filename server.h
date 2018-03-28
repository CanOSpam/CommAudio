#pragma once
#include "ui_server.h"
#include "streamthread.h"
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
    void readData();

private:
    Ui::Server *ui;
    QTcpServer *tcpServer;
    QString fileNames;
    QFileInfoList streamList;
    QList <StreamThread*> *streamThreadList;
};
