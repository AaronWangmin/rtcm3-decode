#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <string>
#include <queue>
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

    queue<string> queAccept;

    void encodeRTCM3(myRTCM3& rtcm3,vector<unsigned char> &rtcm3In);

signals:

public slots:
    void newConnect();
    void sendMessage();
    void acceptMessage();

private:
    QTcpSocket *tcpSocket;

    void byteArray2vector(vector<unsigned char>& destination,QByteArray& source);

};

#endif // MYSOCKET_H
