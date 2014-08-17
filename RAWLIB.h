#ifndef RAWLIB_H
#define RAWLIB_H
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>
#include <stdio.h> //For standard things
#include <string.h>    //memset
#include <arpa/inet.h>
#include <string>
#include <sys/ioctl.h>
#include <net/if.h>
using namespace std;
//#pragma pack(1)


#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server

enum socket_type{
   SOCKET_TYPE_UDP=IPPROTO_UDP,
   SOCKET_TYPE_TCP=IPPROTO_TCP,
   SOCKET_TYPE_ICMP=IPPROTO_ICMP,
   SOCKET_TYPE_IP=IPPROTO_IP,
   SOCKET_TYPE_DNS=999,
};
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t length;
} __attribute__((packed));
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
class RawSocket{
public:
    socket_type type;
    in_addr_t destip;
    in_addr_t sourceip;
    int destport;
    int sourceport;
    int s;
    unsigned short sentid;
    u_int16_t ans_count;
    //char* gateway;
    char* src_mac;
    char* dest_mac;
    RawSocket(char* sourceip,int sourceport,char* destip,int destport,socket_type type);
    void s_send(char* data,int size);
    void s_send(string* data);
    int s_recv(char** data,int size);
    int s_recv(string* data);
    void s_close();
    dnsa* gethostbyname(unsigned char* host,int query_type);
    char* getmac(char* ip);
private:
    void s_send_udp(char* data,int size);
    void s_send_tcp(char* data,int size);
    void s_send_icmp(char* data,int size);
    void s_send_dns(char* data,int size);
    void s_send_ip(char* data,int size);
    void s_send_eth(char*data,int size);

    int s_recv_udp(char** data,int size);
    int s_recv_tcp(char** data,int size);
    int s_recv_icmp(char** data,int size);
    int s_recv_dns(char** data,int size);
    int s_recv_ip(char** data,int size);
    int s_recv_eth(char** data,int size);
};
#endif
