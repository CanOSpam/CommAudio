#include "server.h"

Server::Server(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);

    QDir *dataDir = new QDir(QDir::homePath());
    streamList = dataDir->entryInfoList();
    for(int i = 0; i < streamList.size(); i++)
    {
        if (streamList[i].absoluteFilePath().contains("Music"))
        {
            dataDir = new QDir(streamList[i].absoluteFilePath());
            streamList = dataDir->entryInfoList();
            break;
        }
    }
    for(int i = 0; i < streamList.size(); i++)
    {
        if (streamList[i].absoluteFilePath().contains("Data"))
        {
            dataDir = new QDir(streamList[i].absoluteFilePath());
            streamList = dataDir->entryInfoList();
            break;
        }
    }

    for(int i = 0; i < streamList.size(); i++)
    {
        fileNames += (streamList[i].absoluteFilePath());
        fileNames += ";" ;
    }

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 4242))
    {
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::addClient);
    qDebug("The server is running on\n\nIP: %s\nport: %d\n\n", qPrintable(ipAddress), tcpServer->serverPort());
}

Server::~Server()
{
    delete ui;
    tcpServer->close();
}

Server::addClient()
{
    clientList.append(tcpServer->nextPendingConnection());
    connect(clientList.last(), &QAbstractSocket::disconnected, clientList.last(), &QObject::deleteLater);
    connect(clientList.last(), &QIODevice::readyRead, this, &Server::readData);

    QByteArray header = fileNames.toUtf8();
    header.prepend('0');
    //clientList.last()->write(qPrintable(header));
    header.append('1');
    header.append(';');

    QString ipNames;
    for (int i = 0; i < clientList.size(); i++)
    {
        ipNames += clientList[i]->peerAddress().toString();
        ipNames += ";";
    }
    header += ipNames.toUtf8();
    for (int i = 0; i < clientList.size(); i++)
    {
        clientList[i]->write(qPrintable(header));
    }

    return 0;
}

void Server::readData()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender()); if (socket == 0) return;
    QString data(socket->readAll());
    if (data[0] == '2')
    {
        data = data.remove(0,1);
        for (int i = 0; i < streamList.size(); i++)
        {
            if (streamList[i].absoluteFilePath().contains(data))
            {
                QFile *streamFile = new QFile(streamList[i].absoluteFilePath());
                if (!streamFile->open(QIODevice::ReadOnly))
                {
                    return;
                }

                while(!streamFile->atEnd())
                {
                    QByteArray content = streamFile->readLine();
                    content.prepend('3');
                    socket->write(content);
                }
                streamFile->close();
                break;
            }
        }
    }
}
