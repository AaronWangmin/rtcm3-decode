#ifndef MYRTCM_H
#define MYRTCM_H

#include <string>
using namespace std;

#include <QByteArray>

#include "myhead.h"

class myRtcm
{
public:
    myRtcm();
    bool rtcmFromRaw(QByteArray rawData);

    void parseMsg1004();   // parse msgBuf[1024] by RTCM 1004.   

private:
    // data member
    unsigned char preamble;         // 8 bits,11010011 (0xD3, -45)
    //string reserved;                // 6 bits,not defined -- set to 000000
    unsigned int nMsgLen;           // 10 bits,message length in bytes
    unsigned int nMsgID;            // 12 bits,message type
    unsigned char msgBuf[1024];     // 0-1023 bytes, variable length data message
    unsigned char crcBuf[3];        // 24 bits, QualComm definition CRC-24Q

    RTCM1004 msg1004;

    // member functions
    unsigned int crc_octests(unsigned char *octests,unsigned int len);   // 24 bits CRC checking

    void parseMsg1004Header();      // parse the header of message 1004 from msfBuf[]
    void parseMsg1004Record();      // parse the records of message 1004 from msfBuf[]

    // extract one 1004 message into a vector<unsigned char> which length is 17.
    void extractOneMsg1004();

    // align one 1004 message vector to the first bits.
    // notice: & oneMsg1004 ,perhaps there is some wrong.
    void alignOneMsg1004(vector<unsigned char>& oneMsg1004,int bitsLeft);

    // parse one message 1004.
    void parseOneMsg1004(vector<unsigned char>& oneMsg1004);

    // extract and parse DF012(int20),DF017(int14),DF018(int20)
    int parseDF012Msg1004(vector<unsigned char>& oneMsg1004);
    int parseDF017Msg1004(vector<unsigned char>& onmMsg1004);
    int parseDF018Msg1004(vector<unsigned char>& onmMsg1004);

    // extract a integer from oneMsg1004,and transfer to a int value.
    int parseIntegerMsg1004(vector<unsigned char>& oneMsg1004,
                            int begin,int end,int bitsLeft,int numBits);

    // transfer a vector<unsigned char> to int
    int byteVector2int(vector<unsigned char> byteVector, int numBits);

    void formatOut();
};

#endif // MYRTCM_H
