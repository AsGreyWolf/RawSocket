#include "DnsSocket.h"

DnsSocket::DnsSocket(char* srcip,int srcport,char* dstip,int dstport):UdpSocket(srcip,srcport,dstip,dstport)
{
}
int DnsSocket::s_send(vector<char*> data,vector<int> size,int type,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport)
{
	int ret=0;
	int allsize=sizeof(struct dnshdr);

	for(int i=0; i<size.size(); i++)
		allsize+=size[i];

	char* datagram=new char[allsize];
	struct dnshdr *dnsh;
	memset(datagram, 0, allsize);
	char* senddata = datagram+sizeof(struct dnshdr);

	for(int i=0; i<size.size(); i++)
	{
		memcpy(senddata,data[i],size[i]);
		senddata+=size[i];
	}

	dnsh = (struct dnshdr *)(datagram);
	dnsh->id = (unsigned short) htons(rand()%65536);
	sentid=dnsh->id;
	dnsh->qr = type; //This is a query
	dnsh->opcode = 0; //This is a standard query
	dnsh->aa = 0; //Not Authoritative
	dnsh->tc = 0; //This message is not truncated
	dnsh->rd = 1; //Recursion Desired
	dnsh->ra = 0; //Recursion not available!
	dnsh->z = 0;
	dnsh->ad = 0;
	dnsh->cd = 0;
	dnsh->rcode = 0;
	dnsh->q_count = htons(size.size()); //we have only 1 question
	dnsh->ans_count = 0;
	dnsh->auth_count = 0;
	dnsh->add_count = 0;
	printf("SEND_DNS (%d):\n",allsize);

	for(const char* p = datagram; p<(datagram+allsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	ret=UdpSocket::s_send(datagram,allsize,srcip,srcport,dstip,dstport);
	delete[] datagram;
	return ret;
}
int DnsSocket::s_send(vector<char*> data,vector<int> size,int type)
{
	return s_send(data,size,type,srcip,srcport,dstip,dstport);
}
vector<in_addr> DnsSocket::gethostbyname(unsigned char* host, int query_type)
{
	vector <char*> querydata;
	vector <int> querysize;
	int size=strlen((const char*)host)+1;
	int allsize=size + sizeof(struct dnsq);
	char* datagram=new char[allsize];
	ChangetoDnsNameFormat((unsigned char*)datagram , host);
	dnsq* qinfo;
	qinfo=(struct dnsq*)(datagram+size);
	qinfo->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1); //its internet
	printf("SEND_GETHOSTBYNAME: (%d):\n",allsize);

	for(char* p = datagram; p<(datagram+allsize); ++p)
	{
		printf("%02X ", *p);
	}

	printf("\n");
	querydata.push_back(datagram);
	querysize.push_back(allsize);
	s_send(querydata,querysize,TYPE_DNS_QUERY);
	delete[] datagram;
	vector <char*> answerdata;
	vector <int> answersize;
	int ansvercount=s_recv(answerdata,answersize,TYPE_DNS_ANSWER);
	vector <in_addr> ret;

	for(int i=0; i<ansvercount; i++)
	{
		char* retdata=answerdata[i];
		int retsize=answersize[i];
		printf("RECV_GETHOSTBYNAME: (%d):\n",retsize);

		for(char* p = retdata; p<(retdata+retsize); ++p)
		{
			printf("%02X ", (char)*p);
		}

		printf("\n");
		in_addr retaddr;
		retaddr.s_addr = ((dnsa*)retdata)->aaddr;
		ret.push_back(retaddr);
		delete [] answerdata[i];
	}

	return ret;
}
void DnsSocket::ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
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
int DnsSocket::s_recv(vector<char*>& data,vector<int>& size,int type,int id,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport)
{
	//TODO: catch any type
	char* fulldata=new char[255];
	int newsize=0;

	while(true)
	{
		newsize=UdpSocket::s_recv(fulldata,255,srcip,srcport,dstip,dstport);

		if(((struct dnshdr *)fulldata)->id!=id || (//type!=TYPE_ANY &&
						((struct dnshdr *)fulldata)->qr!=type))
		{}
		else break;
	}

	printf("RECV_DNS (%d):\n",newsize);

	for(char* p = fulldata; p<(fulldata+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	int q_count=ntohs(((struct dnshdr *)fulldata)->q_count);
	int ans_count=ntohs(((struct dnshdr *)fulldata)->ans_count);
	char* b=fulldata;
	b+=sizeof(dnshdr);

	if(type==TYPE_DNS_ANSWER)
		for(int i=0; i<q_count; i++)
		{
			b+=strlen(b)+1+sizeof(dnsq);
		}

	for(int i=0; i<ans_count; i++)
	{
		if(type==TYPE_DNS_QUERY)
			b+=strlen(b)+1;

		int partsize=(type==TYPE_DNS_QUERY?sizeof(dnsq):sizeof(dnsa));
		char* part=new char[partsize];
		memcpy(part,b,partsize);
		data.push_back(part);
		size.push_back(partsize);
		b+=partsize;
	}

	delete[] fulldata;
	return ans_count;
}
int DnsSocket::s_recv(vector<char*>& data,vector<int>& size,int type)
{
	return s_recv(data,size,type,sentid,dstip,dstport,srcip,srcport);
}
