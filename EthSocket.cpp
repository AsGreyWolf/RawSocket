#include "EthSocket.h"

EthSocket::EthSocket(char* srcmac,char* dstmac)
{
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(s < 0)
	{
		cerr<<"Error creating socket"<<endl;
	}

	this->srcmac=srcmac;
	this->dstmac=dstmac;
}
EthSocket::EthSocket(char* dstmac)
{
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(s < 0)
	{
		cerr<<"Error creating socket"<<endl;
	}

	this->dstmac=dstmac;
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "eth0");

	while(true)
	{
		if(ioctl(s, SIOCGIFHWADDR, &ifr) == 0)
			if(ifr.ifr_hwaddr.sa_family == ARPHRD_ETHER) break;
	}

	srcmac=new char[7];
	memcpy(srcmac,ifr.ifr_hwaddr.sa_data,6);
}
EthSocket::EthSocket()
{
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(s < 0)
	{
		cerr<<"Error creating socket"<<endl;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, "eth0");

	while(true)
	{
		if(ioctl(s, SIOCGIFHWADDR, &ifr) == 0)
			if(ifr.ifr_hwaddr.sa_family == ARPHRD_ETHER) break;
	}

	srcmac=new char[7];
	memcpy(srcmac,ifr.ifr_hwaddr.sa_data,6);
	dstmac = getmac("192.168.1.1");
}
char* EthSocket::getmac(char* ip)
{
	ifstream f;
	f.open("/proc/net/arp");
	char ipp[256];
	char* mac=new char[6];
	mac[0]=0;

	while(!f.eof())
	{
		f>>ipp;

		if(strcmp(ipp,ip)==0)
		{
			f>>ipp;
			f>>ipp;
			f>>ipp;
			mac[0]=fromHex(ipp[0])*16;
			mac[0]+=fromHex(ipp[1]);
			mac[1]=fromHex(ipp[3])*16;
			mac[1]+=fromHex(ipp[4]);
			mac[2]=fromHex(ipp[6])*16;
			mac[2]+=fromHex(ipp[7]);
			mac[3]=fromHex(ipp[9])*16;
			mac[3]+=fromHex(ipp[10]);
			mac[4]=fromHex(ipp[12])*16;
			mac[4]+=fromHex(ipp[13]);
			mac[5]=fromHex(ipp[15])*16;
			mac[5]+=fromHex(ipp[16]);
			break;
		}
	}

	f.close();
	return mac;
}
int EthSocket::fromHex(char c)
{
	if(c>='a')
		return c-'a'+10;

	return c-'0';
}
int EthSocket::s_send(char* data,int size,int type,char* srcmac,char* dstmac)
{
	sockaddr_ll socket_address;
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_protocol = htons(type);
	socket_address.sll_ifindex  = 2;
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;
	socket_address.sll_addr[0]  = dstmac[0];
	socket_address.sll_addr[1]  = dstmac[1];
	socket_address.sll_addr[2]  = dstmac[2];
	socket_address.sll_addr[3]  = dstmac[3];
	socket_address.sll_addr[4]  = dstmac[4];
	socket_address.sll_addr[5]  = dstmac[5];
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	int ret=0;
	int allsize=sizeof(ethhdr)+size;
	struct ethhdr *eh;
	char* datagram=new char[allsize];
	memset(datagram, 0, allsize);
	memcpy(datagram + sizeof(struct ethhdr),data,size);
	eh=(struct ethhdr *) datagram;
	memcpy(datagram, dstmac, ETH_ALEN);
	memcpy(datagram+ETH_ALEN, srcmac, ETH_ALEN);
	eh->h_proto=htons(type);
	printf("SEND_ETHERNET (%d):\n",allsize);

	for(const char* p = datagram; p<(datagram+allsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	ret=sendto(s, datagram, allsize, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	delete[] datagram;
	return ret;
}
int EthSocket::s_send(char* data,int size,int type)
{
	return s_send(data,size,type,srcmac,dstmac);
}
int EthSocket::s_recv(char*& data,int type,char* srcfilter,char* dstfilter)
{
	if(lastRecvData!=NULL)
	{
		delete[] lastRecvData;
		lastRecvData=NULL;
	}

	data=new char[ETH_FRAME_LEN];
	lastRecvData=data;
	int newsize=0;

	while(true)
	{
		newsize=recvfrom(s, data, ETH_FRAME_LEN, 0, NULL, NULL);

		if((dstfilter==NULL || memcmp(data,dstfilter,ETH_ALEN)==0) && (srcfilter==NULL || memcmp(data+ETH_ALEN,srcfilter,ETH_ALEN)==0) && (type==TYPE_ANY || ntohs(((ethhdr*)data)->h_proto)==type))
			break;
	}

	printf("RECV_ETH (%d):\n",newsize);

	for(char* p = data; p<(data+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	data+=sizeof(ethhdr);
	newsize-=sizeof(ethhdr);
	return newsize;
}
int EthSocket::s_recv(char*& data,int type)
{
	return s_recv(data,type,dstmac,srcmac);
}
EthSocket::~EthSocket()
{
	if(lastRecvData!=NULL)
	{
		delete[] lastRecvData;
		lastRecvData=NULL;
	}
}
