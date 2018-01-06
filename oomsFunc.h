#pragma 
#include "AESEn_Decrypt.h"
enum PROTOCOL_TYPE
{
	PT_DEFAULT = 0,
	PT_LOGIN_IN = 1,
	PT_DOWNLOAD_PARAM = 2,
	PT_UPDATE_PROGRAM = 3,
	PT_CHECK_TIME = 4,
	PT_SIGN_IN = 5,
	PT_BERTH_INFO = 6,
	PT_BERTH_PUBLISH = 7,
	PT_BERTH_UPLOAD = 8,
	PT_BILL_UPLOAD = 9,
	PT_PAY_UPLOAD = 10,
	PT_QRCODE_REQUST = 11,
	PT_QRCODE_PUBLISH = 12,
	PT_PAY_PUBLISH = 13,
	PT_STATISTICS = 14,
	PT_SIGN_OUT = 15,
	PT_LOGIN_OUT = 16,
	PT_UNPAY_LIST = 17,
	PT_PARKING_LIST = 18,
	PT_RECORD_UPLOAD = 19,
	PT_DEPOSIT_UPLOAD = 20,
	PT_PIC_UPLOAD = 21,
	PT_HEART_BEAT = 22,
	PT_GET_BALANCE = 23,
	PT_RENT_LIST = 24,
	PT_BIND_BALANCE = 25,
	PT_UNBIND_BALANCE = 26,
	PT_CHANGE_PASSWORD = 27,
	PT_SUSPECT_LIST = 28,
	PT_CHANGE_STATE = 29,
	PT_VIDEO = 30,
	PT_GET_INSPECTOR = 31,
	PT_INSPECTOR_UPDATE = 32,
	PT_SINGLE_BERTH = 33,
	PT_INSPECTOR_PUBLISH = 34,
	PT_BERTH_INOUT_PUBLISH = 35,
	PT_CHECK_SUSPECT = 36
};

#define HEAD      (char)'!'    //°üÍ·
#define TAIL_1    (char)0xAB   //°üÎ²
#define TAIL_2    (char)0xAA
#define CONST_LEN 12     //HEAD(1)+CMD(1)+SEQ(2)+DATALEN(4)+CRC(2)+TAIL(2) 

unsigned short crc16_ccitt(unsigned char *buf, unsigned short len);
std::string BuildPacket(const char* data, int datalen, BYTE cmd);
std::string Convert(const char* strIn, int sourceCodepage, int targetCodepage);
bool CheckPacket(const char* packet, int packetlen);
std::string ExtractPacket(const char* packet, int packetlen);

