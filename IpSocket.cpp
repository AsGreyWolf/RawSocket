#include "IpSocket.h"

IpSocket::IpSocket(char* srcip,char* dstip):EthSocket()
{
	this->dstip=inet_addr(dstip);
	this->srcip=inet_addr(srcip);
	srand(time(0));
}
unsigned short IpSocket::csum(unsigned short *ptr,int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;
	sum=0;

	while(nbytes>1)
	{
		sum+=*ptr++;
		nbytes-=2;
	}

	if(nbytes==1)
	{
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	return(answer);
}
int IpSocket::s_send(char* data,int datasize,int type,in_addr_t srcip,in_addr_t dstip)
{
	int id= (unsigned short) htons(rand()%65536);
	struct iphdr *iph;
	int maxsize=ETH_DATA_LEN-sizeof(iphdr);
	int smesh=0;
	int ret=0;

	while(smesh<datasize)
	{
		int size=datasize-smesh;
		int flags=0;

		if(size>maxsize)
		{
			size=maxsize;
			flags=8192;
		}

		int allsize=sizeof(struct ip)+size;
		char* datagram=new char[allsize];
		memset(datagram, 0, allsize);
		memcpy(datagram + sizeof(struct iphdr),data,size);
		iph=(struct iphdr *) datagram;
		iph->ihl = 5;
		iph->version = 4;
		iph->tos = 0;
		iph->tot_len = htons(allsize);
		flags+=smesh/8;
		iph->frag_off = htons(flags);
		iph->ttl = 255;
		iph->protocol = type;
		iph->check = 0;
		iph->saddr = srcip;
		iph->daddr = dstip;
		iph->id =id; //Id of this packet
		iph->check = csum((unsigned short *) iph, sizeof(struct iphdr));
		printf("SEND_IP (%d):\n",allsize);

		for(const char* p = datagram; p<(datagram+allsize); ++p)
		{
			printf("%02X ", (char)*p);
		}

		printf("\n");
		ret+=EthSocket::s_send(datagram,allsize,TYPE_IP);
		delete[] datagram;
		smesh+=size;
	}

	return ret;
}
int IpSocket::s_send(char* data,int size,int type)
{
	return s_send(data,size,type,srcip,dstip);
}
int IpSocket::s_recv(char*& fulldata,int size,int type,in_addr_t srcip,in_addr_t dstip)
{
	int newsize=0;
	int id=-1;
	char* data;

	while(true)
	{
		int packetsize=0;
		packetsize=EthSocket::s_recv(data,TYPE_IP);
		iphdr* iph=(iphdr*)data;

		if(iph->version == 4 &&
					(type==TYPE_ANY || (iph->protocol == type && iph->saddr == srcip && iph->daddr == dstip)) &&
					(iph->id ==id || id==-1)) {}
		else
		{
			continue;
		}

		id=iph->id;
		printf("RECV_IP (%d):\n",packetsize);

		for(char* p = data; p<(data+packetsize); ++p)
		{
			printf("%02X ", (char)*p);
		}

		printf("\n");
		int hasnext=ntohs(iph->frag_off) & 8192;
		int smesh=(ntohs(iph->frag_off) & 8191)*8;
		data+=sizeof(iphdr);
		packetsize-=sizeof(iphdr);
		newsize=min(max(smesh+packetsize,newsize),size);
		memcpy(fulldata+smesh,data,min(packetsize,newsize-smesh));

		if(hasnext == 0 || newsize==size) break;
	}

	return newsize;
}
int IpSocket::s_recv(char*& fulldata,int size,int type)
{
	return s_recv(fulldata,size,type,dstip,srcip);
}
