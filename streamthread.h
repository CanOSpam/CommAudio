#ifndef STREAMTHREAD_H
#define STREAMTHREAD_H
#include <QtNetwork>

class StreamThread : public QThread
{

public:
    StreamThread(QTcpSocket *tcpsocket, QFileInfo dir);
    void run();

private:
    QTcpSocket *clientSocket;
    QFileInfo streamDir;

};

#endif // STREAMTHREAD_H
