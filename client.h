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
    void streamStateChange();
    void displayError(QAbstractSocket::SocketError socketError);
    int peerConnRequest();

private slots:
    void on_playButton_clicked();

    void on_connectButton_clicked();

    void on_pauseButton_clicked();

private:
    Ui::Client *ui;
    QTcpSocket *tcpSocket = nullptr;
    QTcpSocket *peerSocket;
    QTcpServer *tcpServer;

    QByteArray data;
    QString filename;

    QAudioOutput* audio;
    QAudioFormat format;
    bool streaming = false;
    bool paused = false;
};
