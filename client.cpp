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
    QString data(tcpSocket->readAll());
    if (data[0] == '0')
    {
        data = data.remove(0,1);
        QList<QString> streamList = data.split(QRegExp(";|/"));
        for (int i = 0; i <streamList.size(); i++)
        {
            if (streamList[i].contains(".mp3"))
            {
                ui->streamComboBox->addItem(streamList[i]);
            }
        }
    }
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

void Client::on_playButton_clicked()
{
    QString header = "1";
    header.append(ui->streamComboBox->currentText());
    tcpSocket->write(qPrintable(header));
}
