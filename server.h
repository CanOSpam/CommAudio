#pragma once
#include "ui_server.h"
#include <QtNetwork>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>

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
    QFile file;
    QAudioOutput* audio;

};
