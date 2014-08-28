#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "IpSocket.h"


class UdpSocket : public IpSocket
{
public:
	UdpSocket(char* srcip,int srcport,char* dstip,int dstport);

	int s_send(char* data,int size,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport);
	int s_send(char* data,int size);
	int s_recv(char*& data,int size,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport);
	int s_recv(char*& data,int size);
protected:
	int srcport;
	int dstport;
private:
	struct pseudo_header
	{
		u_int32_t source_address;
		u_int32_t dest_address;
		u_int8_t placeholder;
		u_int8_t protocol;
		u_int16_t length;
	} __attribute__((packed));
};

#endif // UDPSOCKET_H
