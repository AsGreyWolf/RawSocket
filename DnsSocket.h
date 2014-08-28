#ifndef DNSSOCKET_H
#define DNSSOCKET_H

#include "UdpSocket.h"
#include <vector>

#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server

class DnsSocket : public UdpSocket
{
public:
	DnsSocket(char* srcip,int srcport,char* dstip,int dstport);

	int s_send(vector<char*> data,vector<int> size,int type,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport);
	int s_send(vector<char*> data,vector<int> size,int type);
	int s_recv(vector<char*>& data,vector<int>& size,int type,int id,in_addr_t srcip,int srcport,in_addr_t dstip,int dstport);
	int s_recv(vector<char*>& data,vector<int>& size,int type);

	vector<in_addr> gethostbyname(unsigned char* host, int query_type);
	void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host);

	static const int TYPE_DNS_QUERY=0;
	static const int TYPE_DNS_ANSWER=1;
protected:
private:
	int sentid;
	struct dnsa
	{
		u_int16_t aname;
		u_int16_t atype;
		u_int16_t aclass;
		u_int32_t attl;
		u_int16_t alength;
		u_int32_t aaddr;
	} __attribute__((packed));
	struct dnsq
	{
		u_int16_t qtype;
		u_int16_t qclass;
	} __attribute__((packed));
	struct dnshdr
	{
		u_int16_t id; // identification number

		u_int8_t rd :1; // recursion desired
		u_int8_t tc :1; // truncated message
		u_int8_t aa :1; // authoritive answer
		u_int8_t opcode :4; // purpose of message
		u_int8_t qr :1; // query/response flag

		u_int8_t rcode :4; // response code
		u_int8_t cd :1; // checking disabled
		u_int8_t ad :1; // authenticated data
		u_int8_t z :1; // its z! reserved
		u_int8_t ra :1; // recursion available

		u_int16_t q_count; // number of question entries
		u_int16_t ans_count; // number of answer entries
		u_int16_t auth_count; // number of authority entries
		u_int16_t add_count; // number of resource entries
	} __attribute__((packed));
};

#endif // DNSSOCKET_H
