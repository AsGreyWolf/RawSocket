#include "UdpSocket.h"
#include <netinet/udp.h>

UdpSocket::UdpSocket(char* srcip,int srcport,char* dstip,int dstport):IpSocket(srcip,dstip)
{
	this->srcport=srcport;
	this->dstport=dstport;
}
int UdpSocket::s_send(char* data,int size,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport)
{
	int ret=0;
	int allsize=sizeof(struct udphdr)+size;
	char* datagram=new char[allsize];
	struct udphdr *udph;
	memset(datagram, 0, allsize);
	memcpy(datagram + sizeof(struct udphdr),data,size);
	udph = (struct udphdr *)(datagram);
	udph->source = htons(srcport);
	udph->dest = htons(dstport);
	udph->len = htons(sizeof(struct udphdr) + size);
	udph->check = 0;
	struct pseudo_header psh;
	psh.source_address = srcip;
	psh.dest_address = dstip;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.length = htons(allsize);
	int psize=sizeof(struct pseudo_header)+allsize;
	char* pseudogram = (char*)malloc(psize);
	memcpy(pseudogram , (char*) &psh , sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , udph , allsize);
	udph->check = csum((unsigned short*) pseudogram ,psize);
	printf("SEND_UDP (%d):\n",allsize);

	for(const char* p = datagram; p<(datagram+allsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	ret=IpSocket::s_send(datagram,allsize,TYPE_UDP,srcip,dstip);
	delete[] datagram;
	return ret;
}
int UdpSocket::s_send(char* data,int size)
{
	return s_send(data,size,srcip,srcport,dstip,dstport);
}
int UdpSocket::s_recv(char*& data,int size,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport)
{
	int newsize=0;
	char* fulldata=new char[size+sizeof(udphdr)];

	while(true)
	{
		newsize=IpSocket::s_recv(fulldata,size+sizeof(udphdr),TYPE_UDP,srcip,dstip);

		if((srcport<0 || ntohs(((struct udphdr *)fulldata)->source)!=srcport) ||
					(dstport<0 || ntohs(((struct udphdr *)fulldata)->dest)!=dstport))
		{}
		else break;
	}

	printf("RECV_UDP (%d):\n",newsize);

	for(char* p = fulldata; p<(fulldata+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	newsize-=sizeof(udphdr);
	memcpy(data,fulldata+sizeof(udphdr),newsize);
	delete[] fulldata;
	return newsize;
}
int UdpSocket::s_recv(char*& data,int size)
{
	return s_recv(data,size,dstip,dstport,srcip,srcport);
}
