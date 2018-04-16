#include "client.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readData);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::displayError);

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
    connect(audio, &QAudioOutput::stateChanged, this, &Client::streamStateChange);

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


    bool ok = false;
    QString ipAddr = QInputDialog::getText(this, tr("IP Address"),
                                                 tr("IP to connect to:"), QLineEdit::Normal,
                                                 "0.0.0.0", &ok);

    tcpSocket->connectToHost(QHostAddress(ipAddr),4242);
}

Client::~Client()
{
    delete ui;
}

void Client::readData()
{
    if (!streaming && !multicast)
    {
        // List of choices for client
        data = tcpSocket->peek(1);
        if (data[0] == '0')
        {
            data = tcpSocket->read(1);
            data = tcpSocket->readAll();
            QString* list = new QString(data);
            QList<QString> streamList = list->split(QRegExp(";"));
            for (int i = 0; i <streamList.size(); i++)
            {
                if (streamList[i].contains("/"))
                {
                    QList<QString> sList = streamList[i].split("/");
                    for (int i = 0; i < sList.size(); i++)
                    {
                        if (sList[i].contains(".wav"))
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
        else
        {
            ui->playButton->setEnabled(false);
            ui->listenButton->setEnabled(false);
            data = tcpSocket->readAll();
            if (!downloadFile->isOpen())
            {
                if (!downloadFile->open(QIODevice::WriteOnly | QIODevice::Append))
                        return;
            }
            downloadFile->write(data);

            downloadFile->close();
            ui->playButton->setEnabled(true);
            ui->listenButton->setEnabled(true);
        }
    }
    else if (streaming || multicast)
    {
        if ((audio->state() == QAudio::IdleState || audio->state() == QAudio::StoppedState))
        {
            audio->start(tcpSocket);
        }
    }
}

void Client::streamStateChange()
{
    if (audio->state() != QAudio::ActiveState && tcpSocket->bytesAvailable() == 0 && !multicast)
    {
        ui->downloadButton->setEnabled(true);
        ui->listenButton->setEnabled(true);
        streaming = false;
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
    ui->downloadButton->setEnabled(false);
    ui->listenButton->setEnabled(false);
    if (streaming)
    {
        audio->resume();
    }
    else
    {
        QString header = "2";
        header.append(ui->streamComboBox->currentText());
        filename = ui->streamComboBox->currentText();
        streaming = true;
        paused = false;
        tcpSocket->write(qPrintable(header));
    }
}

void Client::on_connectButton_clicked()
{
    QString ipText = ui->ipComboBox->currentText();
    tcpSocket->connectToHost(QHostAddress(ipText),4242);
}

int Client::peerConnRequest()
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

        //QAudioInput *audio = new QAudioInput(format, this);
        audio->setBufferSize(1024);
        //audio->start(socket);
    }
    return 0;
}

void Client::on_pauseButton_clicked()
{
    if (streaming)
    {
        audio->suspend();
    }
}

void Client::on_stopButton_clicked()
{
    ui->downloadButton->setEnabled(true);
    ui->listenButton->setEnabled(true);
    if (streaming)
    {
        audio->stop();
        tcpSocket->readAll();
        streaming = false;
    }
}

void Client::on_downloadButton_clicked()
{
    ui->playButton->setEnabled(false);
    ui->listenButton->setEnabled(false);
    OutputFolder = QFileDialog::getExistingDirectory(0, ("Select Output Folder"), QDir::currentPath());
    QString header = "3";
    header.append(ui->streamComboBox->currentText());
    filename = ui->streamComboBox->currentText();
    QFile* file = new QFile(OutputFolder.absolutePath() + "//" + filename);
    if (file->isOpen())
    {
        file->close();
    }
    if (file->exists())
    {
        file->remove();
    }
    downloadFile = new QFile(OutputFolder.absolutePath() + "//" + filename);
    if (!downloadFile->open(QIODevice::WriteOnly | QIODevice::Append))
            return;

    tcpSocket->write(qPrintable(header));
}

void Client::on_stopLocalButton_clicked()
{
    ui->downloadButton->setEnabled(true);
    ui->playButton->setEnabled(true);
    if (localPlaying)
    {
        player->stop();
        localPlaying = false;
    }
}

void Client::on_pauseLocalButton_clicked()
{
    if (localPlaying)
    {
        player->pause();
    }
}

void Client::on_playLocalButton_clicked()
{
    ui->downloadButton->setEnabled(false);
    ui->playButton->setEnabled(false);
    if (localPlaying)
    {
        player->play();
    }
    else
    {
        QString name = QFileDialog::getOpenFileName(this,
            tr("Open File"), "/", tr("Song Files (*.mp3 *.wav)"));

        localPlaying = true;
        player = new QMediaPlayer;
        player->setMedia(QUrl::fromLocalFile(name));
        player->setVolume(50);
        player->play();

    }
}

void Client::on_listenButton_clicked()
{
    if (!streaming && !localPlaying)
    {
        multicast = true;
        QString header = "4";
        tcpSocket->write(qPrintable(header));
        ui->playButton->setEnabled(false);
        ui->downloadButton->setEnabled(false);
    }
}

void Client::on_stopListenButton_clicked()
{
    if (!streaming && !localPlaying)
    {
        multicast = false;
        QString header = "5";
        tcpSocket->write(qPrintable(header));
        ui->playButton->setEnabled(true);
        ui->downloadButton->setEnabled(true);

        audio->stop();
        tcpSocket->readAll();
    }
}
