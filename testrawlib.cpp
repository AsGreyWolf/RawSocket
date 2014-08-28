#include "DnsSocket.h"
#include <stdio.h>
DnsSocket* s;
int main()
{
	s=new DnsSocket("192.168.1.3",8303,"192.168.1.1",53);
	unsigned char data[100];
	memcpy(data,(unsigned char*)"www.google.com.",15);
	vector<in_addr>val=s->gethostbyname(data,T_A);

	for(int i=0; i<val.size(); i++)
	{
		printf("Answer:\n");
		printf("ip:%s\n",inet_ntoa(val[i]));
	}

	return 0;
}
