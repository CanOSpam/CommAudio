#pragma once
#include "ui_client.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>

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

private slots:
    void on_playButton_clicked();

private:
    Ui::Client *ui;
    QTcpSocket *tcpSocket = nullptr;
    QByteArray data;
    QFile fileWrite;
    QFile fileRead;
    QAudioOutput* audio;
    bool newFile = true;
};
