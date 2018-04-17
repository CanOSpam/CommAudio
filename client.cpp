#include "client.h"

/*------------------------------------------------------------------------------------------------------------------
   -- SOURCE FILE: client.cpp
   --
   -- PROGRAM: CommAudio
   --
   -- FUNCTIONS:
   -- void Client::readData()
   -- void Client::streamStateChange()
   -- void Client::displayError(QAbstractSocket::SocketError socketError)
   -- void Client::on_playButton_clicked()
   -- void Client::on_connectButton_clicked()
   -- void Client::on_disconnectButton_clicked()
   -- int Client::peerConnRequest()
   -- void Client::readVoice()
   -- void Client::on_stopButton_clicked()
   -- void Client::on_downloadButton_clicked()
   -- void Client::on_stopLocalButton_clicked()
   -- void Client::on_pauseLocalButton_clicked()
   -- void Client::on_playLocalButton_clicked()
   -- void Client::on_listenButton_clicked()
   -- void Client::on_stoplistenButton_clicked()
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
   -- The server component of CommAudio. This file contains all functions required to run the client to send and Receive
   -- audio files from the corresponding server side of the program.
   ----------------------------------------------------------------------------------------------------------------------*/
Client::Client(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::Client)
        , tcpSocket(new QTcpSocket(this))
        , peerSocketOut(new QTcpSocket(this))
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: ~Client (destructor)
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
   -- INTERFACE: Client::~Client()
   --
   -- RETURNS: N/A
   --
   -- NOTES:
   -- Deletes the client UI.
   ----------------------------------------------------------------------------------------------------------------------*/
Client::~Client()
{
        delete ui;
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
   -- PROGRAMMER: Haley Booker
   --
   -- INTERFACE: void Client::readData()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Reads data from the server transmitting audio. Also handles enabling and disabling of UI elements based
   -- on current player state.
   ----------------------------------------------------------------------------------------------------------------------*/
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
                                        for (int j = i; j < streamList.size() - 1; j++)
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: streamStateChange
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
   -- INTERFACE: void Client::streamStateChange()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Enables UI elements when streaming is done.
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::streamStateChange()
{
        if (audio->state() != QAudio::ActiveState && tcpSocket->bytesAvailable() == 0 && !multicast)
        {
                ui->downloadButton->setEnabled(true);
                ui->listenButton->setEnabled(true);
                streaming = false;
        }
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: displayError
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
   -- INTERFACE: void Client::displayError(QAbstractSocket::SocketError socketError)
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Displays an error message for a given socket error. If error does not patch pre set errors, prints that errors
   -- errorString.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_playButton_clicked
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
   -- INTERFACE: void Client::on_playButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- When the play button is pressed, disable audio stat controls. If streaming mode, audio playback is resumed.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_connectButton_clicked
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker
   --
   -- PROGRAMMER: Haley Booker, Tim Bruecker, Alfred Swinton
   --
   -- INTERFACE: void Client::on_connectButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Configure audio player sample rate and size. Set to stereo audio. Set codec. Connect to server.
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::on_connectButton_clicked()
{
        format.setChannelCount(2);
        format.setSampleRate(44100);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);

        audioin = new QAudioInput(format,this);

        //peerSocketOut->disconnectFromHost();
        QString ipText = ui->ipComboBox->currentText();
        peerSocketOut->connectToHost(QHostAddress(ipText),8484, QIODevice::WriteOnly);
        audioin->start(peerSocketOut);
        qDebug() << "Client Connected";
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_disconnectButton_clicked
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
   -- INTERFACE: void Client::on_disconnectButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Prints a debug message that the client has disconnected.
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::on_disconnectButton_clicked()
{
        // Check if the socket is connected if it is disconnect
        qDebug() << "Client Disconnected";
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: peerConnRequest
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Haley Booker
   --
   -- PROGRAMMER: Haley Booker, Mac Craig, Alfred Swinton
   --
   -- INTERFACE: int Client::peerConnRequest()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- If the peer socket doesn't exist, create it for the next connection. Set the sample rate, stereo audio, and codec.
   -- Connect to the newly created peer socket.
   ----------------------------------------------------------------------------------------------------------------------*/
int Client::peerConnRequest()
{
        if (peerSocket == nullptr)
        {
                peerSocket = tcpServer->nextPendingConnection();

                format.setChannelCount(2);
                format.setSampleRate(44100);
                format.setSampleSize(16);
                format.setCodec("audio/pcm");
                format.setByteOrder(QAudioFormat::LittleEndian);
                format.setSampleType(QAudioFormat::SignedInt);

                audioout = new QAudioOutput(format,this);
                connect(peerSocket, &QIODevice::readyRead, this, &Client::readVoice);
        }
        return 0;
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: readVoice
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER: Alfred Swinton
   --
   -- PROGRAMMER: Alfred Swinton
   --
   -- INTERFACE: void Client::readVoice()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- If audio output is available, start outputting from the peer socket created in peerConnRequest().
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::readVoice()
{
        if ((audioout->state() == QAudio::IdleState || audioout->state() == QAudio::StoppedState))
        {
                audioout->start(peerSocket);
        }
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_pauseButton_clicked
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
   -- INTERFACE: void Client::on_pauseButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Stop audio playback.
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::on_pauseButton_clicked()
{
        if (streaming)
        {
                audio->suspend();
        }
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_stopButton_clicked
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
   -- INTERFACE: void Client::on_stopButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Re-enable audio play options, stop streaming of audio.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_downloadButton_clicked
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
   -- INTERFACE: void Client::on_downloadButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Disable audio playback buttons, choose download location, open a new file, write socket contents to file.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_stopLocalButton_clicked
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
   -- INTERFACE: void Client::on_stopLocalButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Re-enable play audio buttons, stop audio playback.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_pauseLocalButton_clicked
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
   -- INTERFACE: void Client::on_pauseLocalButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Pause audio playback.
   ----------------------------------------------------------------------------------------------------------------------*/
void Client::on_pauseLocalButton_clicked()
{
        if (localPlaying)
        {
                player->pause();
        }
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_playLocalButton_clicked
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
   -- INTERFACE: void Client::on_playLocalButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Disable audio play buttons, begin audio playback if currently playing, else pick a new file to play, start playing.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_listenButton_clicked
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
   -- INTERFACE: void Client::on_listenButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Set multicast option to true, disable audio playback buttons.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: on_stopListenButton_clicked
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
   -- INTERFACE: void Client::on_stopListenButton_clicked()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Disable multi casting, re-enable audio playback buttons, stop audio playback and clear tcp socket.
   ----------------------------------------------------------------------------------------------------------------------*/
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
