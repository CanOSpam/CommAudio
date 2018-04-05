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
    data = tcpSocket->readAll();
    if (data[0] == '0')
    {
        QString *list = new QString(data);
        QString fileString = list->remove(0,1);
        QList<QString> streamList = fileString.split(QRegExp(";|/"));
        for (int i = 0; i <streamList.size(); i++)
        {
            if (streamList[i].contains(".wav") || streamList[i].contains(".mp3"))
            {
                ui->streamComboBox->addItem(streamList[i]);
            }
        }
    }

    if (data[0] == '2')
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
    QString header = "1";
    header.append(ui->streamComboBox->currentText());
    tcpSocket->write(qPrintable(header));
}
