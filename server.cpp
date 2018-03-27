#include "server.h"

Server::Server(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);

    QDir *dataDir = new QDir(QDir::homePath());
    QFileInfoList streamList = dataDir->entryInfoList();
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
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected, clientConnection, &QObject::deleteLater);
    qDebug("Client connected.\n");
    clientConnection->write(qPrintable(fileNames));
    return 0;
}
