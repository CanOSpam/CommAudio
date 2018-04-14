#pragma once
#include "ui_client.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>
#include <QtMultimedia/QMediaPlayer>
#include <QSound>

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
    peerConnRequest();

private slots:
    void on_playButton_clicked();

    void on_connectButton_clicked();

private:
    Ui::Client *ui;
    QTcpSocket *tcpSocket = nullptr;
    QTcpSocket *peerSocket;
    QTcpServer *tcpServer;

    QByteArray data;
    QFile fileWrite;
    QFile fileRead;
    QString filename;

    QAudioOutput* audio;
    QAudioFormat format;
    bool streaming = false;
};
