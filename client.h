#pragma once
#include "ui_client.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>
#include <QMessageBox>

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
    int peerConnRequest();

private slots:
    void on_playButton_clicked();

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void ConnectBack();

    void RunMessageBox(QString ipAddress);

    void speaking();

private:
    Ui::Client *ui;
    QTcpSocket *tcpSocket = nullptr;
    QTcpSocket *peerSocketOut = nullptr;
    QTcpSocket *peerSocket = nullptr;
    QTcpServer *tcpServer;
    QByteArray data;
    QFile fileWrite;
    QFile fileRead;
    QAudioOutput* audio;
    bool newFile = true;
};
