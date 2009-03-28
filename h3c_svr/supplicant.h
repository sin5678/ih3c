#define _vsnprintf vsnprintf
#include <pcap.h>
#include <iphlpapi.h>
#include "md5_wrapper.h"

#include "resource.h"

#define PCAP_OPENFLAG_PROMISCUOUS 1
//定义用户信息结构
typedef struct UserData
{
	char username[20];
	char password[20];
	unsigned char Ip[4];
	unsigned char Mac[6];
	char nic[60];
}USERDATA;

void GetNIC( int nicIndex);

void GetMacAddrFromIP(const char *strIP,unsigned char *Mac);


void FillDestMac(const unsigned char *strDstMAC);    // 填充各种包的Destination MAC 
void FillSrcMac(const  unsigned char *strSrcMAC);    // 填充各种包的Source MAC

void FillIP(const unsigned char *strIP);
 
void FillUserName(const char *strUsername);


void FillUserNameID(const unsigned char *strUserNameID);  // 填充发送用户名包的ID
void FillSessionID(const unsigned char *strSessionID);   // 填充对话维持包的ID
void FillPasswdID(const unsigned char *strUserNameID);   // 填充发送密码包的ID


void SetMd5Buf(const u_char *ID, const u_char *chap);    //MD5 加密算法,产生加密后的回复密文


BOOL Connect();
BOOL DisConnect();

BOOL OpenDevice( const char *device);
BOOL CloseDevice();

BOOL StartSupplicant(); 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

void encodeVersion();
