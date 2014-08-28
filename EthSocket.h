#ifndef ETHSOCKET_H
#define ETHSOCKET_H
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fstream>
using namespace std;

class EthSocket
{
public:
	//TODO: get default gateway
	EthSocket(char* srcmac,char* dstmac);
	EthSocket(char* dstmac);
	EthSocket();
	~EthSocket();

	char* getmac(char* ip);
	int fromHex(char c);

	int s_send(char* data,int size,int type,char* srcmac,char* dstmac);
	int s_send(char* data,int size,int type);
	int s_recv(char*& data,int type,char* srcfilter,char* dstfilter);
	int s_recv(char*& data,int type);

	static const int TYPE_IP=ETH_P_IP;
	static const int TYPE_ANY=-999;
protected:
	char* srcmac;
	char* dstmac;
private:
	int s;
	char* lastRecvData;
};

#endif // ETHSOCKET_H
