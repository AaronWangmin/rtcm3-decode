#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <string>
#include <vector>
using namespace std;

#include <QObject>
#include <QTcpSocket>

using namespace std;

#include "myrtcm1004.h"

class mySocket : public QObject
{
    Q_OBJECT
public:
    explicit mySocket(QObject *parent = 0);  
    
signals:

public slots:
    void newConnect();
    void sendMessage();
    void acceptMessage();

private:
    QTcpSocket *tcpSocket;

};

#endif // MYSOCKET_H
