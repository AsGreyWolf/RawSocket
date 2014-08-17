#include "RAWLIB.h"
#include <stdio.h>
RawSocket* s;
int main(){
    s=new RawSocket("192.168.1.1",80,"8.8.8.8",53,SOCKET_TYPE_DNS);
    unsigned char data[100];
    memcpy(data,(unsigned char*)"www.google.com.",15);
	dnsa* answer=s->gethostbyname(data,T_A);
    int newsize=sizeof(dnsa)*s->ans_count;
    printf("resolver (%d):\n",newsize);

	for(char* p = (char*)answer; p<( (char*)answer+newsize); ++p)
	{
		printf("%02X ", (char)*p);
	}

	printf("\n");
	struct in_addr ip_addr;
    ip_addr.s_addr = answer->aaddr;
    for(int i=0;i<s->ans_count;i++){
		printf("Answer:\n");
		printf("Name:%u, type:%u, class:%u, ttl:%u, length:%u, ip:%s\n",htons(answer->aname),htons(answer->atype),htons(answer->aclass),(answer->attl),htons(answer->alength),inet_ntoa(ip_addr));
		answer++;
	}
    return 0;
}
