#ifndef IPSOCKET_H
#define IPSOCKET_H

#include "EthSocket.h"
#include <netinet/ip.h>


class IpSocket : public EthSocket
{
	public:
	//TODO: get src ip
		IpSocket(char* srcip,char* dstip);

		unsigned short csum(unsigned short *ptr,int nbytes);

		int s_send(char* data,int size,int type);
		int s_send(char* data,int size,int type,in_addr_t srcip,in_addr_t dstip);
		int s_recv(char*& data,int size,int type,in_addr_t srcip,in_addr_t dstip);
		int s_recv(char*& data,int size,int type);

		static const int TYPE_UDP=IPPROTO_UDP;
		static const int TYPE_TCP=IPPROTO_TCP;
	protected:
		in_addr_t srcip;
		in_addr_t dstip;
	private:
};

#endif // IPSOCKET_H
