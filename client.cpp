#include "client.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
    , tcpSocket(new QTcpSocket(this))
    , peerSocketOut(new QTcpSocket(this))
{
    ui->setupUi(this);
    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readData);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::displayError);

    tcpSocket->abort();

    // The clients server for peer requests
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 8484))
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
    connect(tcpServer, &QTcpServer::newConnection, this, &Client::peerConnRequest);
    qDebug("The client server is running on\n\nIP: %s\nport: %d\n\n", qPrintable(ipAddress), tcpServer->serverPort());

    tcpSocket->connectToHost(QHostAddress("192.168.0.24"),4242);
}

Client::~Client()
{
    delete ui;
}


void Client::readData()
{
    data = tcpSocket->readAll();
    if (data[0] == '0')
    {
        QString *list = new QString(data);
        QString fileString = list->remove(0,1);
        QList<QString> streamList = fileString.split(QRegExp(";"));
        for (int i = 0; i <streamList.size(); i++)
        {
            if (streamList[i].contains("/"))
            {
                QList<QString> sList = streamList[i].split("/");
                for (int i = 0; i < sList.size(); i++)
                {
                    if (sList[i].contains(".wav") || sList[i].contains(".mp3"))
                    {
                        ui->streamComboBox->addItem(sList[i]);
                    }
                }
            }
            else if (streamList[i].compare("1"))
            {
                for (int j = i; j < streamList.size() - 1;j++)
                {
                    ui->ipComboBox->addItem(streamList[j]);
                }
                break;
            }

        }
    }

    else if (data[0] == '3')
    {
        data = data.remove(0,1);
        fileWrite.setFileName("./test.wav");
        if(!fileWrite.isOpen())
        {
            fileWrite.open(QIODevice::WriteOnly | QIODevice::Append);
        }
        fileWrite.write(data);
        fileWrite.close();
        if (newFile)
        {
            newFile = false;
            fileRead.setFileName("./test.wav");
            if(!fileRead.isOpen())
            {
                fileRead.open(QIODevice::ReadOnly);
            }
            QAudioFormat format;
            format.setSampleRate(44100);
            format.setChannelCount(1);
            format.setSampleSize(16);
            format.setCodec("audio/pcm");
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::SignedInt);

            QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
            if (!info.isFormatSupported(format)) {
                qWarning() << "Default format not supported - trying to use nearest";
                format = info.nearestFormat(format);
            }
            audio = new QAudioOutput(format, this);
            audio->start(&fileRead);
            fileRead.close();
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
    QString header = "2";
    header.append(ui->streamComboBox->currentText());
    tcpSocket->write(qPrintable(header));
}

void Client::on_connectButton_clicked()
{
    peerSocketOut->disconnectFromHost();
    //if (peerSocket->state() == QAbstractSocket::ConnectedState)
    //{
    //    peerSocket->disconnectFromHost();
    //}
    QString ipText = ui->ipComboBox->currentText();
    peerSocketOut->connectToHost(QHostAddress(ipText),8484);
    qDebug() << "Client Connected";
}

void Client::on_disconnectButton_clicked()
{
    // Check if the socket is connected if it is disconnect
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {

    }
    RunMessageBox("test");
    qDebug() << "Client Disconnected";
}

int Client::peerConnRequest()
{
    if (peerSocket == NULL)
    {
        peerSocket = tcpServer->nextPendingConnection();
        connect(peerSocket, &QAbstractSocket::disconnected, peerSocket, &QObject::deleteLater);
        connect(peerSocket, &QIODevice::readyRead, this, &Client::readData);
    }
    return 0;
}

// Message Box When A client connects peer to peer for microphone
void Client::RunMessageBox(QString ipAddress)
{
    QMessageBox acceptConn;
    acceptConn.setText("You have received a p2p microphone request from: " + ipAddress);
    acceptConn.setInformativeText("Accept Connection?");
    acceptConn.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    if (acceptConn.exec() == QMessageBox::Ok)
    {
        qDebug() << "Ok Pressed!";
        ConnectBack();
    }
    else if (acceptConn.exec() == QMessageBox::Cancel)
    {
        qDebug() << "Cancel Pressed!";
    }
}

// Connects back to the client if the client agrees
void Client::ConnectBack()
{
    qDebug() << "Connected Back!";
}


void Client::on_speakButton_clicked()
{
    QMessageBox speak;
    speak.setText("2 way microphone enabled speak now!");
    speak.setStandardButtons(QMessageBox::Cancel);
    speak.exec();
    while (true)
    {

    }
}
