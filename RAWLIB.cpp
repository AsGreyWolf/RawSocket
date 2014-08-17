#include "RAWLIB.h"
#include <fstream>
unsigned short csum(unsigned short *ptr,int nbytes)
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
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
	int lock = 0 , i;


	for(i = 0 ; i < strlen((char*)host) ; i++)
	{
		if(host[i]=='.')
		{
			*dns++ = i-lock;

			for(; lock<i; lock++)
			{
				*dns++=host[lock];
			}

			lock++;
		}
	}

	*dns++='\0';
}
RawSocket::RawSocket(char* sourceip,int sourceport,char* destip,int destport,socket_type type)
{
	srand(time(0));
	this->type=type;
	this->destport=destport;
	this->destip=inet_addr(destip);
	this->sourceip=inet_addr(sourceip);
	this->sourceport=sourceport;
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(s < 0)
	{
		cerr<<"Error creating socket"<<endl;
	}

	int one = 1;
	const int *val = &one;
	/* if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	 {
	     cerr<<"Error setting IP_HDRINCL"<<endl;
	 }*/
	//sleep(1);
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "eth0");

	while(true)
	{
		if(ioctl(s, SIOCGIFHWADDR, &ifr) == 0)
			if(ifr.ifr_hwaddr.sa_family == ARPHRD_ETHER) break;
	}

	src_mac=new char[7];
	//dest_mac=new char[7];
	memcpy(src_mac,ifr.ifr_hwaddr.sa_data,6);
	dest_mac = getmac("192.168.1.1");
	printf("\n");
};
void RawSocket::s_close()
{
	close(s);
};
void RawSocket::s_send(char* data,int size)
{
	switch(type)
	{
	case SOCKET_TYPE_IP:
		s_send_ip(data,size);
		break;

	case SOCKET_TYPE_UDP:
		s_send_udp(data,size);
		break;

	case SOCKET_TYPE_ICMP:
		s_send_icmp(data,size);
		break;

	case SOCKET_TYPE_DNS:
		s_send_dns(data,size);
		break;

	case SOCKET_TYPE_TCP:
		s_send_tcp(data,size);
		break;
	}
}
void RawSocket::s_send_eth(char* data,int size)
{
	//if(size<60)size=60;
	int allsize=sizeof(ethhdr)+size;
	struct ethhdr *eh;
	sockaddr_ll socket_address;
	/*RAW communication*/
	socket_address.sll_family   = PF_PACKET;
	/*we don't use a protocoll above ethernet layer
	  ->just use anything here*/
	socket_address.sll_protocol = htons(ETH_P_IP);
	/*index of the network device
	see full code later how to retrieve it*/
	socket_address.sll_ifindex  = 2;
	/*ARP hardware identifier is ethernet*/
	socket_address.sll_hatype   = ARPHRD_ETHER;
	/*target is another host*/
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	/*address length*/
	socket_address.sll_halen    = ETH_ALEN;
	socket_address.sll_addr[0]  = dest_mac[0];
	socket_address.sll_addr[1]  = dest_mac[1];
	socket_address.sll_addr[2]  = dest_mac[2];
	socket_address.sll_addr[3]  = dest_mac[3];
	socket_address.sll_addr[4]  = dest_mac[4];
	socket_address.sll_addr[5]  = dest_mac[5];
	socket_address.sll_addr[6]  = 0x00;/*not used*/
	socket_address.sll_addr[7]  = 0x00;/*not used*/
	char* datagram=new char[allsize];
	memset(datagram, 0, allsize);
	char* senddata = datagram + sizeof(struct ethhdr);
	memcpy(senddata,data,size);
	eh=(struct ethhdr *) datagram;
	memcpy((void*)datagram, (void*)dest_mac, ETH_ALEN);
	memcpy((void*)(datagram+ETH_ALEN), (void*)src_mac, ETH_ALEN);
	eh->h_proto =htons(ETH_P_IP);

	 printf("SEND_ETHERNET (%d):\n",allsize);
	 for (const char* p = datagram; p<(datagram+allsize); ++p)
	 {
	     printf("%02X ", (char)*p);
	 }
	 printf("\n");
	if(sendto(s, datagram, allsize, 0, (struct sockaddr*)&socket_address, sizeof(socket_address))<0)
	{
		cout<<"Send error"<<endl;
	}
	delete[] datagram;
}

void RawSocket::s_send_ip(char* data,int datasize)
{
	int id= (unsigned short) htons(rand()%65536);
	struct iphdr *iph;
	int maxsize=ETH_DATA_LEN-sizeof(iphdr);
	//struct sockaddr_in sin;
	// sin.sin_family = AF_INET;
	//sin.sin_port = htons(destport);
	//sin.sin_addr.s_addr = destip;
	int smesh=0;

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
		char* senddata = datagram + sizeof(struct iphdr);
		memcpy(senddata,data,size);
		iph=(struct iphdr *) datagram;
		iph->ihl = 5;
		iph->version = 4;
		iph->tos = 0;
		iph->tot_len = htons(allsize);
		flags+=smesh/8;
		iph->frag_off = htons(flags);
		iph->ttl = 255;

		if(type!=SOCKET_TYPE_DNS)
			iph->protocol = type;
		else
			iph->protocol = SOCKET_TYPE_UDP;

		iph->check = 0;
		iph->saddr = sourceip;
		iph->daddr = destip;
		iph->id =id; //Id of this packet
		iph->check = csum((unsigned short *) iph, sizeof(struct iphdr));
		printf("SEND_IP (%d):\n",allsize);
		for (const char* p = datagram; p<(datagram+allsize); ++p)
		{
		    printf("%02X ", (char)*p);
		}
		printf("\n");
		s_send_eth(datagram,allsize);
		delete[] datagram;
		smesh+=size;
	}
}
void RawSocket::s_send_udp(char* data,int size)
{
	int allsize=sizeof(struct udphdr)+size;
	char* datagram=new char[allsize];
	struct udphdr *udph;
	memset(datagram, 0, allsize);
	char* senddata = datagram + sizeof(struct udphdr);
	memcpy(senddata,data,size);
	udph = (struct udphdr *)(datagram);
	udph->source = htons(sourceport);
	udph->dest = htons(destport);
	udph->len = htons(sizeof(struct udphdr) + size);
	udph->check = 0;
	struct pseudo_header psh;
	psh.source_address = sourceip;
	psh.dest_address = destip;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.length = htons(allsize);
	int psize=sizeof(struct pseudo_header)+allsize;
	char* pseudogram = (char*)malloc(psize);
	memcpy(pseudogram , (char*) &psh , sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , udph , allsize);
	udph->check = csum((unsigned short*) pseudogram ,psize);
	printf("SEND_UDP (%d):\n",allsize);
	for (const char* p = datagram; p<(datagram+allsize); ++p)
	{
	    printf("%02X ", (char)*p);
	}
	printf("\n");
	s_send_ip(datagram,allsize);
	delete[] datagram;
}
void RawSocket::s_send_tcp(char* data,int size) {}
void RawSocket::s_send_icmp(char* data,int size) {}
void RawSocket::s_send_dns(char* data,int size)
{
	int allsize=sizeof(struct dnshdr)+size;
	char* datagram=new char[allsize];
	struct dnshdr *dnsh;
	memset(datagram, 0, allsize);
	char* senddata = datagram + sizeof(struct dnshdr);
	memcpy(senddata,data,size);
	dnsh = (struct dnshdr *)(datagram);
	dnsh->id = (unsigned short) htons(rand()%65536);
	sentid=dnsh->id;
	dnsh->qr = 0; //This is a query
	dnsh->opcode = 0; //This is a standard query
	dnsh->aa = 0; //Not Authoritative
	dnsh->tc = 0; //This message is not truncated
	dnsh->rd = 1; //Recursion Desired
	dnsh->ra = 0; //Recursion not available!
	dnsh->z = 0;
	dnsh->ad = 0;
	dnsh->cd = 0;
	dnsh->rcode = 0;
	dnsh->q_count = htons(1); //we have only 1 question
	dnsh->ans_count = 0;
	dnsh->auth_count = 0;
	dnsh->add_count = 0;
	printf("SEND_DNS (%d):\n",allsize);
	for (const char* p = datagram; p<(datagram+allsize); ++p)
	{
	    printf("%02X ", (char)*p);
	}
	printf("\n");
	s_send_udp(datagram,allsize);
	delete[] datagram;
}
dnsa* RawSocket::gethostbyname(unsigned char* host, int query_type)
{
	unsigned char* buf=new unsigned char[255];
	unsigned char *qname;
	qname=(unsigned char*)buf;
	ChangetoDnsNameFormat(qname , host);
	dnsq* qinfo;
	qinfo =(struct dnsq*)&buf[(strlen((char*)qname) + 1)];
	qinfo->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1); //its internet
	int allsize=(strlen((const char*)qname)+1) + sizeof(struct dnsq);
	 printf("SEND_GETHOSTBYNAME: (%d):\n",allsize);
	 for (unsigned char* p = buf; p<(buf+allsize); ++p)
	 {
	     printf("%02X ", (char)*p);
	 }
	 printf("\n");
	s_send_dns((char*)buf,allsize);
	delete[] buf;
	/*char** data=new char*();
    int newsize=s_recv(data,999);
    printf("RECV_GETHOSTBYNAME: (%d):\n",newsize);

	for(char* p = *data; p<(*data+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");

	newsize-=strlen(*data)+1;
	(*data)+=strlen(*data)+1;

	(*data)+=sizeof(dnsq);
	newsize-=sizeof(dnsq);

    //s_close();
    char* cc=(*data);
    delete data;
    return (dnsa*)(cc);*/
    return NULL;
}
int RawSocket::s_recv(char** data,int size)
{
	switch(type)
	{
	case SOCKET_TYPE_IP:
		return s_recv_ip(data,size);
		break;

	case SOCKET_TYPE_UDP:
		return s_recv_udp(data,size);
		break;

	case SOCKET_TYPE_ICMP:
		return s_recv_icmp(data,size);
		break;

	case SOCKET_TYPE_DNS:
		return s_recv_dns(data,size);
		break;

	case SOCKET_TYPE_TCP:
		return s_recv_tcp(data,size);
		break;
	}
}

int RawSocket::s_recv_udp(char** data,int size)
{
	int newsize=0;

	while(true)
	{
		newsize=s_recv_ip(data,size);

		if(ntohs(((struct udphdr *)(*data))->source)!=destport ||
					ntohs(((struct udphdr *)(*data))->dest)!=sourceport)
			delete[](*data);
		else break;
	}

	printf("RECV_UDP (%d):\n",newsize);

	for(char* p = *data; p<(*data+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	(*data)+=sizeof(udphdr);
	newsize-=sizeof(udphdr);
	return newsize;
}
int RawSocket::s_recv_tcp(char** data,int size) {}
int RawSocket::s_recv_icmp(char** data,int size) {}
int RawSocket::s_recv_dns(char** data,int size)
{
	int newsize=0;
	while(true)
	{
		newsize=s_recv_udp(data,size);
		if(((struct dnshdr *)(*data))->id!=sentid)
			delete[](*data);
		else break;
	}

	printf("RECV_DNS (%d):\n",newsize);

	for(char* p = *data; p<(*data+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	ans_count=htons(((struct dnshdr *)(*data))->ans_count);
	(*data)+=sizeof(dnshdr);
	newsize-=sizeof(dnshdr);
	return newsize;
}
int RawSocket::s_recv_ip(char** fulldata,int size)
{
	(*fulldata)=new char[size];
	int newsize=0;
	int id=-1;
	char** data=new char*();

	while(true)
	{
		int packetsize=0;
		packetsize=s_recv_eth(data,size);
		iphdr* iph=(iphdr*)(*data);

		if(iph->version == 4 &&
					((type==SOCKET_TYPE_DNS && iph->protocol == SOCKET_TYPE_UDP) || iph->protocol == type) &&
					iph->saddr == destip &&
					iph->daddr == sourceip &&
					(iph->id ==id || id==-1)) {}
		else
		{
			*data-=sizeof(ethhdr);
			delete[](*data);
			continue;
		}

		id=iph->id;
		printf("RECV_IP (%d):\n",packetsize);

		for(char* p = *data; p<(*data+packetsize); ++p)
		{
			printf("%02X ", (char)*p);
		}

		printf("\n");
		int hasnext=ntohs(iph->frag_off) & 8192;
		int smesh=(ntohs(iph->frag_off) & 8191)*8;
		*data+=sizeof(iphdr);
		packetsize-=sizeof(iphdr);
		memcpy((*fulldata)+smesh,*data,packetsize);
		newsize=max(smesh+packetsize,newsize);
		*data-=sizeof(iphdr)+sizeof(ethhdr);
		delete[](*data);

		if(hasnext == 0) break;
	}

	delete data;
	return newsize;
}
int RawSocket::s_recv_eth(char** data,int size)
{
	(*data)=new char[ETH_FRAME_LEN];
	int newsize=0;

	while(memcmp(*data,src_mac,ETH_ALEN)!=0 || memcmp(*data+ETH_ALEN,dest_mac,ETH_ALEN)!=0)
	{
		newsize=recvfrom(s, *data, ETH_FRAME_LEN, 0, NULL, NULL);
	}

	printf("RECV_ETH (%d):\n",newsize);

	for(char* p = *data; p<(*data+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	/*if(ntohs(((struct ethhdr *)data)->h_proto)>=ETH_ZLEN-ETH_HLEN){
	    data+=3;
	    newsize-=7;
	}*/
	*data+=sizeof(ethhdr);
	newsize-=sizeof(ethhdr);
	return newsize;
}
int fromHex(char c)
{
	if(c>='a')
		return c-'a'+10;

	return c-'0';
}
char* RawSocket::getmac(char* ip)
{
	ifstream f;
	f.open("/proc/net/arp");
	char ipp[256];
	char* mac=new char[256];
	mac[0]=0;

	while(!f.eof())
	{
		f>>ipp;

		//printf("%s\n",ipp);
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
