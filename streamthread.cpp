#include "streamthread.h"

StreamThread::StreamThread(QTcpSocket *tcpsocket, QFileInfo dir)
    : clientSocket(tcpsocket)
    , streamDir(dir)
{

}

//The function to stream the audio
void StreamThread::run()
{
}
