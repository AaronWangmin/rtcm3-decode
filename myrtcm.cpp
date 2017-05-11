#include "myrtcm.h"

#include <iostream>

using namespace std;

myRtcm::myRtcm()
{
    // data member initialize
    preamble = 0xFF;
    nMsgLen = 0;
    nMsgID = 0;
}

bool myRtcm::rtcmFromRaw(QByteArray rawData)
{
   QByteArray::Iterator it;
   for(it = rawData.begin();it != rawData.end();it++){
       unsigned char cFirst = *it;
       if( 0xD3 == (cFirst & 0xFF) ){
//           cout << " premble is correct !" << endl;

           unsigned char cTemp1 = *(++it);
           // check reserved 6 bits == 0, 0xFC = 111111 00
           if( 0 != (cTemp1 & 0xFC) ){
               cout <<  "the reserved 6 bits is wrong ! " << endl;
               continue;
           }

           unsigned char cTemp2 = *(++it);
           // read message length
           nMsgLen = (cTemp1 & 0x03)*256 + (cTemp2 & 0xFF);

           msgBuf[0] = cFirst;
           msgBuf[1] = cTemp1;
           msgBuf[2] = cTemp2;

           // read message contents
           for(unsigned int j = 0; j < nMsgLen; j++){
               msgBuf[j+3] = *(++it);
           }

           // read CRC-24 parity
           for(unsigned int j = 0; j < 3; j++){
              crcBuf[j] = *(++it);
           }

           // perform CRC check
           unsigned int crc_calculated = 0;
           crc_calculated = crc_octests(msgBuf,nMsgLen + 3);
           if(crc_calculated == (crcBuf[0] << 16) + (crcBuf[1] << 8) + crcBuf[2] ){
//               cout << " crc checked correct ! " << endl;

               // parse the detail of RTCM
               nMsgID = msgBuf[3+0]*16 + (msgBuf[3+1] & 0xF0)/16;

               switch (nMsgID) {
               case 1004:
                   parseMsg1004();
                   break;
               case 1006:
                   cout << " 1006 message !" << endl;
                   break;
               case 1104:
                   cout << " 1104 message !" << endl;
                   break;
               default:
                   cout << " other message !" << endl;
                   break;
               }
           }

           return true;
       }
   }

   cout << " not a rtcm message !" << endl;
   return false;
}

unsigned int myRtcm::crc_octests(unsigned char *octests, unsigned int len){
    unsigned int CRC_INIT = 0x000000;
    unsigned int CRC24_POLY = 0x1864cfb;

    unsigned int crc = CRC_INIT;           // initial to 0
    int i;
    while(len--){
        crc ^= (*octests++) << 16;
        for(i = 0; i < 8; i++){
            crc <<= 1;
            if( crc & 0x1000000) crc ^= CRC24_POLY;
        }
    }
    return crc & 0xFFFFFF;
}

void myRtcm::parseMsg1004(){
    cout << " 1004 message: " << endl;
    parseMsg1004Header();
    parseMsg1004Record();
}

void myRtcm::parseMsg1004Header()
{
    // parse the RTCM1004 Header, total 64 bits, msgBuf[3+0] ~ msgBuf[3+7]
    // msg1004.header.msgID = msgBuf[3+0]*16 + (msgBuf[3+1] & 0xF0)/16;

    msg1004.header.staID =( msgBuf[3+1] & 0x0F)*256 + msgBuf[3+2];
    msg1004.header.TOW = msgBuf[3+3]*4194304 + msgBuf[3+4]*16384 + msgBuf[3+5]*64
            + ( msgBuf[3+6] & 0xFC)/4;
    msg1004.header.flagGPSSyn = ( msgBuf[3+5] & 0x02 )/2;
    msg1004.header.numGPSProcessed = ( msgBuf[3+5] & 0x01)*16 + (msgBuf[3+7] & 0xF0)/16;
    msg1004.header.indicatorGPSSmooth = (msgBuf[3+7] & 0x08)/8;
    msg1004.header.intervalGPSSmooth = (msgBuf[3+7] & 0x07);

    // test:
    cout << "  Station ID: " << msg1004.header.staID << endl;
    cout << "         TOW: " << msg1004.header.TOW << endl;
    cout << " SynGNSSFlag: " << msg1004.header.flagGPSSyn << endl;
    cout << " numGNSSProc: " << msg1004.header.numGPSProcessed << endl;
    cout << "indicatorSmo: " << msg1004.header.indicatorGPSSmooth << endl;
    cout << "intervalSmo0: " << msg1004.header.intervalGPSSmooth << endl;
}

void myRtcm::parseMsg1004Record()
{
    extractOneMsg1004();
}


void myRtcm::extractOneMsg1004()
{
    int start = 10;     // the last byte of the 1004 message header.
    int bitsLeft = 0;
    for(unsigned int i = 0; i < msg1004.header.numGPSProcessed; i++){
        if(bitsLeft >= 8){
           bitsLeft -= 8;
           --start;
        }

        vector<unsigned char> oneMsg1004;
        for(int j = 0;j < 17;j++){
            oneMsg1004.push_back(msgBuf[start + j]);
        }

        alignOneMsg1004(oneMsg1004,bitsLeft);
        parseOneMsg1004(oneMsg1004);

        bitsLeft += 3;
        start += 16;
    }
}

void myRtcm::alignOneMsg1004(vector<unsigned char>& oneMsg1004,int bitsLeft)
{
    if(bitsLeft != 0){
       // note: don't process the last byte, perhaps will result to some error !
        for(unsigned int i = 0; i < oneMsg1004.size() -1;i++){
           unsigned char newCurrent = ( oneMsg1004.at(i) << (8 - bitsLeft) ) |      // ? if change the raw oneMsg1004
                   ( oneMsg1004.at(i+1) >> bitsLeft );
           oneMsg1004.at(i) = newCurrent;
       }
   }
}

void myRtcm::parseOneMsg1004(vector<unsigned char> &oneMsg1004)
{
    RTCM1004Record rtcm1004Record;
    rtcm1004Record.GPSID = ( oneMsg1004.at(0) & 0xFC)/4;
    rtcm1004Record.indicatorL1Code = ( oneMsg1004.at(0) & 0x02)/2;
    rtcm1004Record.L1Pseud = ( oneMsg1004.at(0) & 0x01) * 8388608 +
                               oneMsg1004.at(1) * 32768 +
                               oneMsg1004.at(2) * 128 +
                             ( oneMsg1004.at(3) & 0xFE) /2;
    rtcm1004Record.L1Phase_L1Pseud = parseDF012Msg1004(oneMsg1004);
//    parseDF012Msg1004(oneMsg1004);

    rtcm1004Record.L1LockTime = ( oneMsg1004.at(6) & 0x1F) * 4 +
                                ( oneMsg1004.at(7) & 0xC0) /64;
    rtcm1004Record.IntegerPseudAmbiguity_L1 =  ( oneMsg1004.at(7) & 0x3F) * 4 +
                                               ( oneMsg1004.at(8) & 0xC0) /64;
    rtcm1004Record.L1CNR = ( oneMsg1004.at(8) & 0x3F) * 4 +
                           ( oneMsg1004.at(9) & 0xC0) /64;

    rtcm1004Record.indicatorL2Code = ( oneMsg1004.at(9) & 0x30)/64;
    rtcm1004Record.L2_L1PseudDiff = parseIntegerMsg1004(oneMsg1004,9,11,4,14);
    rtcm1004Record.L2Phase_L1Pseud = parseIntegerMsg1004(oneMsg1004,11,13,6,20);
    rtcm1004Record.L2LockTime = ( oneMsg1004.at(13) & 0x03) * 32 +
                                ( oneMsg1004.at(14) & 0xF8)/8;
    rtcm1004Record.L2CNR = ( oneMsg1004.at(14) & 0x07) * 32 +
                           ( oneMsg1004.at(15) & 0xF8)/8;


    msg1004.record.push_back(rtcm1004Record);

    // test:
//    cout << "            GPSID: " << rtcm1004Record.GPSID << endl;
//    cout << "  indicatorL1Code: " << rtcm1004Record.indicatorL1Code << endl;
//    cout << "          L1Pseud: " << rtcm1004Record.L1Pseud << endl;
//    cout << " L1Phase_L1Pseud : " << rtcm1004Record.L1Phase_L1Pseud << endl;
//    cout << "       L1LockTime: " << rtcm1004Record.L1LockTime << endl;
//    cout << "IntegerPseudAmbiguity_L1: " << rtcm1004Record.IntegerPseudAmbiguity_L1 << endl;
//    cout << "            L1CNR: " << rtcm1004Record.L1CNR << endl;

//    cout << "  indicatorL2Code: " << rtcm1004Record.indicatorL2Code << endl;
//    cout << "   L2_L1PseudDiff: " << rtcm1004Record.L2_L1PseudDiff << endl;
//    cout << "  L2Phase_L1Pseud: " << rtcm1004Record.L2Phase_L1Pseud << endl;
//    cout << "       L2LockTime: " << rtcm1004Record.L2LockTime << endl;
//    cout << "            L2CNR: " << rtcm1004Record.L2CNR << endl;
    cout.width(2) ;
    cout << rtcm1004Record.GPSID << "   "
         << rtcm1004Record.indicatorL1Code << "   ";
    cout.width(8);
    cout << rtcm1004Record.L1Pseud << "    ";
    cout.width(7);
    cout << rtcm1004Record.L1Phase_L1Pseud  << "    ";
    cout.width(3);
    cout << rtcm1004Record.L1LockTime << "    ";
    cout.width(3);
    cout << rtcm1004Record.IntegerPseudAmbiguity_L1 << "    ";
    cout.width(3);
    cout << rtcm1004Record.L1CNR << endl;
}

int myRtcm::parseDF012Msg1004(vector<unsigned char>& oneMsg1004)
{
    vector<unsigned char> DF012;
    for(int i = 3; i <= 6; i++)
        DF012.push_back(oneMsg1004.at(i));

    alignOneMsg1004(DF012,1);
    return byteVector2int(DF012,20);
}

int myRtcm::parseIntegerMsg1004(vector<unsigned char> &oneMsg1004,
                                int begin, int end, int bitsLeft,int numBits)
{
    vector<unsigned char> iDF;
    for(int i = begin; i <= end; i++){
        iDF.push_back(oneMsg1004.at(i));
    }

    alignOneMsg1004(iDF,bitsLeft);
    return byteVector2int(iDF,numBits);
}

int myRtcm::byteVector2int(vector<unsigned char> byteVector,int numBits)
{
    // process int20,3 bytes
    unsigned int temp = 0;
    switch (numBits){
    case 20:
        temp = byteVector.at(0) * 4096 +
                            byteVector.at(1) * 16 +
                            ( byteVector.at(2) & 0xF0 )/16;
        return (int)temp;
        break;
    case 14:
        temp = byteVector.at(0) * 64 +
                            ( byteVector.at(1) & 0xFC )/4;
        return (int)temp;
        break;
    default:
        cout << " the value is not 20bits or 14 bits !" << endl;
        break;

    }
}
