#pragma once
#include "ui_server.h"
#include <QtNetwork>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QDataStream>
#include <QFileDialog>

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
int addClient();
void readData();

private slots:
void on_startMultiButton_clicked();

private:
Ui::Server *ui;
QTcpServer *tcpServer;
QString fileNames;
QList<QTcpSocket*> clientList;
QList<QTcpSocket*> multicastList;
QList<QDataStream*> dataStreamList;
QFileInfoList streamList;
QDataStream* dataStream;
};
