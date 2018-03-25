#include "client.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readData);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::displayError);

    tcpSocket->abort();
    tcpSocket->connectToHost(QHostAddress("192.168.56.1"),4242);
}

Client::~Client()
{
    delete ui;
}

void Client::readData()
{

}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
       qDebug("The host was not found. Please check the host name and port settings.\n");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug("The connection was refused by the peer.\n");
        break;
    default:
        qDebug("The following error occurred: %s.", qPrintable(tcpSocket->errorString()));
    }
}
