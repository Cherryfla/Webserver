#include<cstdio>
#include<cstdlib>
#include<errno.h>
#include<cstring>
#include<netdb.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<netinet/in.h>
#define PORT 8088 /* 客户机连接远程主机的端口 */
#define MAXDATASIZE 1024 /* 每次可以接收的最大字节 */
int main(int argc,char *argv[]){
	int sockfd,numbytes;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */
	if(argc!=2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}
	if((he=gethostbyname(argv[1]))==NULL){ /* get the host info */		
		herror("gethostbyname");
		exit(1);
	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) {
		perror("socket");
		exit(1);
	}
	their_addr.sin_family=AF_INET; /* host byte order */
	their_addr.sin_port=htons(PORT); /* short, network byte order */
	their_addr.sin_addr=*((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero),sizeof(their_addr)); /* zero the rest of the struct */
	if(connect(sockfd,(struct sockaddr *)&their_addr,sizeof(sockaddr))== -1){
		perror("connect");
		exit(1);
	}
	char buff[MAXDATASIZE]="GET /html/index.html HTTP/1.1";
	send(sockfd,buff,sizeof(buff),0);
	if((numbytes=recv(sockfd,buf,MAXDATASIZE,0))==-1){
		perror("recv");
		exit(1);
	}
	buf[numbytes]='\0';
	printf("Received: %s",buf);
	close(sockfd);
	return 0;
}