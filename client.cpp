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

    format.setSampleRate(96000);
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

    tcpSocket->connectToHost(QHostAddress("192.168.56.1"),4242);
}

Client::~Client()
{
    delete ui;
}

void Client::readData()
{
    if (!streaming)
    {
        // List of choices for client
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

        // File data to play
        else if (data[0] == '3')
        {
            streaming = true;
    //        data = data.remove(0,1);
    //        if(!fileWrite.isOpen())
    //        {
    //            fileWrite.open(QIODevice::ReadWrite | QIODevice::Append);
    //        }
    //        fileWrite.write(data);
        }
        //End of file
    //    else if (data[0] == '4')
    //    {
    //        fileWrite.close();
    //        fileRead.setFileName("./" + filename);
    //        if(!fileRead.isOpen())
    //        {
    //            fileRead.open(QIODevice::ReadOnly);
    //            qDebug("Opening fileRead\n");
    //        }

    //        audio = new QAudioOutput(format, this);
    //        audio->start(&fileRead);
    //    }
    }
    else
    {
//        fileWrite.write(data);
        if (audio->state() == QAudio::IdleState || audio->state() == QAudio::StoppedState)
        {
            audio->start(tcpSocket);
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
    filename = ui->streamComboBox->currentText();
    fileWrite.setFileName("./" + filename);

    tcpSocket->write(qPrintable(header));
}

void Client::on_connectButton_clicked()
{
    QString ipText = ui->ipComboBox->currentText();
    tcpSocket->connectToHost(QHostAddress(ipText),4242);
}

Client::peerConnRequest()
{
    if (peerSocket == NULL)
    {
        peerSocket = tcpServer->nextPendingConnection();
        connect(peerSocket, &QAbstractSocket::disconnected, peerSocket, &QObject::deleteLater);
        connect(peerSocket, &QIODevice::readyRead, this, &Client::readData);

        QAudioFormat format;
        format.setChannelCount(1);
        format.setSampleRate(8000);
        format.setSampleSize(8);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioInput *audio = new QAudioInput(format, this);
        audio->setBufferSize(1024);
        //audio->start(socket);
    }
    return 0;
}
