#pragma once
#include "ui_client.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>
#include <QMessageBox>
#include <QtMultimedia/QMediaPlayer>
#include <QSound>
#include <QFileDialog>
#include <QInputDialog>

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

void on_disconnectButton_clicked();

void on_pauseButton_clicked();

void on_stopButton_clicked();

void on_downloadButton_clicked();

void on_stopLocalButton_clicked();

void on_pauseLocalButton_clicked();

void on_playLocalButton_clicked();

void on_listenButton_clicked();

void on_stopListenButton_clicked();

void readVoice();

private:
Ui::Client *ui;
QTcpSocket *tcpSocket = nullptr;
QTcpSocket *peerSocketOut = nullptr;
QTcpSocket *peerSocket = nullptr;
QTcpServer *tcpServer;

QByteArray data;
QString filename;
QDir OutputFolder;
QFile* downloadFile;

QAudioOutput* audio;
QAudioFormat format;
qint64 fileSize;

QMediaPlayer* player;

QAudioInput * audioin;
QAudioOutput * audioout;

bool streaming = false;
bool localPlaying = false;
bool multicast = false;
bool paused = false;
};
