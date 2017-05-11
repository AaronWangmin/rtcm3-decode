#include "mysocket.h"

#include <iostream>
#include <QDataStream>
#include <QHostAddress>
#include <vector>
#include <QDebug>

#include "myhex.h"
#include "myrtcm.h"

using namespace std;

mySocket::mySocket(QObject *parent) : QObject(parent)
{ 
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(acceptMessage()));

}

void mySocket::newConnect()
{
    tcpSocket->abort();
    tcpSocket->connectToHost(QHostAddress("175.102.134.67"),12102);
//    tcpSocket->connectToHost(QHostAddress::LocalHost,60000);
}

void mySocket::sendMessage()
{
    newConnect();

    // for NTRIP, the two message blow must to be send
    tcpSocket->write("GET /TJ3N HTTP/1.0");
    tcpSocket->write("Authorization: Basic MTAwMDAwMWplcnJ5OmE=");

    // the two message below is not necessary
    tcpSocket->write("ser-Agent: NTRIP rtcm test");
    tcpSocket->write("Accept:?*/*");

    // base64 encode, (user:password)
//  cout << (QByteArray("100000jerry:a").toBase64()).toStdString() << endl;
//  tcpSocket->write("Authorization: Basic " + QByteArray("100000jerry:a").toBase64());
}

void mySocket::acceptMessage()
{
    sendMessage();

    while(tcpSocket->waitForReadyRead()){        

        QByteArray recv =  tcpSocket->readAll() ;       // QByteArray read data from socket.

        // After success to the authority, then send the NMEA message and receiver the source data.

        tcpSocket->write("$GPGGA,110514.00,3054.1935,N,12147.9475,E,1,10,0.947,41.790,M,,M,0,*70");

        cout << "------  rtcmFromRaw test ------ \n" << endl;
        myRtcm rtcm;
        rtcm.rtcmFromRaw(recv);

    }
}
