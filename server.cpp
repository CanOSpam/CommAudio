#include "server.h"

/*------------------------------------------------------------------------------------------------------------------
   -- SOURCE FILE: server.cpp
   --
   -- PROGRAM: CommAudio
   --
   -- FUNCTIONS:
   -- int Server::addClient()
   -- void Server::readData()
   -- void Server::on_startMultiButton_clicked()
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   --
   -- DESIGNER: Haley Booker, Mackenzie Craig, Tim Bruecker, Alfred Swinton
   --
   -- PROGRAMMER: Haley Booker, Mackenzie Craig, Tim Bruecker, Alfred Swinton
   --
   -- NOTES:
   -- The server component of CommAudio. This file contains all functions required to run the server to send and Receive
   -- audio files from the corresponding client side of the program.
   ----------------------------------------------------------------------------------------------------------------------*/
Server::Server(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::Server)
{
        ui->setupUi(this);

        QDir dataDir = QDir(QFileDialog::getExistingDirectory(0, "Music Directory", QDir::currentPath()));
        streamList = dataDir.entryInfoList();

        for(int i = 0; i < streamList.size(); i++)
        {
                fileNames += (streamList[i].absoluteFilePath());
                fileNames += ";";
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: ~Server (destructor)
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker
   --
   -- PROGRAMMER: Haley Booker
   --
   -- INTERFACE: Server::~Server()
   --
   -- RETURNS: N/A
   --
   -- NOTES:
   -- Deletes the client UI, closes the tcp connection.
   ----------------------------------------------------------------------------------------------------------------------*/
Server::~Server()
{
        delete ui;
        tcpServer->close();
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: addClient
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker
   --
   -- PROGRAMMER: Haley Booker, Alfread Swinton
   --
   -- INTERFACE: int Server::addClient()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Adds the new client to the list of current clients, add client IP to list, send client header.
   ----------------------------------------------------------------------------------------------------------------------*/
int Server::addClient()
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: readData
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker
   --
   -- PROGRAMMER: Haley Booker, Tim Bruecker
   --
   -- INTERFACE: void Server::readData()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Function checks message type:
   -- 2: Receive from client.
   -- 3: Send to client.
   -- 4: Add client to "multicast" group.
   -- 5: Remove client from "multicast" group.
   ----------------------------------------------------------------------------------------------------------------------*/
void Server::readData()
{
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender()); if (socket == 0) return;
        QString data = socket->readAll();
        qDebug() << "data: " << data;
        if (data[0] == '2')
        {
                data = data.remove(0,1);
                for (int i = 0; i < streamList.size(); i++)
                {
                        if (streamList[i].absoluteFilePath().contains(data))
                        {
                                QFile *streamFile = new QFile(streamList[i].absoluteFilePath());
                                if (!streamFile->isOpen())
                                {
                                        if (!streamFile->open(QIODevice::ReadOnly))
                                        {
                                                return;
                                        }
                                }
                                dataStreamList.append(new QDataStream(socket));

                                QByteArray content = streamFile->readAll();
                                *dataStreamList.last() << content;
                                streamFile->close();
                                break;
                        }
                }
        }
        else if (data[0] == '3')
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

                                while (!streamFile->atEnd())
                                {
                                        QByteArray content = streamFile->readLine();
                                        socket->write(content);
                                }
                                streamFile->close();
                                break;
                        }
                }
        }
        else if (data[0] == '4')
        {
                multicastList.append(socket);
                qDebug() << "Added " << socket;
        }
        else if (data[0] == '5')
        {
                for (int i = 0; i < multicastList.size(); i++)
                {
                        if (multicastList[i]->localAddress().isEqual(socket->localAddress()))
                        {
                                qDebug() << "(5) Removing: " << multicastList[i];
                                multicastList.removeAt(i);
                        }
                }
        }
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_startMultiButton_clicked
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker, Mac Craig
   --
   -- PROGRAMMER: Haley Booker, Mac Craig
   --
   -- INTERFACE: void Server::on_startMultiButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Starts a multicast with a selected song. Read file into byte array, write to each client in "multicast" list.
   ----------------------------------------------------------------------------------------------------------------------*/
void Server::on_startMultiButton_clicked()
{
        QString name = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), "/", tr("Song Files (*.wav)"));

        QFile *streamFile = new QFile(name);
        if (!streamFile->isOpen())
        {
                if (!streamFile->open(QIODevice::ReadOnly))
                {
                        return;
                }
        }

        QByteArray content = streamFile->readAll();

        for (int i = 0; i < multicastList.size(); i++)
        {
                qDebug() << multicastList.size();
                multicastList[i]->write(content);
        }
        streamFile->close();
}
