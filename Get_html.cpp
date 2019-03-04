#include<cstdio>
#include<sys/socket.h>
#include<sys/types.h>
#define PORT 8848
#define	BACKLOG 10
using namespace std;

int main(){
	int sockfd=socket(AT_INET,SOCK_STREAM,0);//建立套接字
	if(sockfd==-1){
		perror("Socket");
		exit(1);
	}
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);			//转化为网络字节序
	addr.sin_addr.s_addr=INADDR_ANY； 	//初始化为我的IP
	bzero(&(my_addr.sin_zero));			//多余的字节初始为0
	
	//将本地端口和套接字绑定
	if(bind(sock,(const struct sockaddr*)&address,sizeof(address))==-1){
		perror("bind");
		exit(1);
	}

	if(listen(sock,BACKLOG)==-1){	//第二个参数是等待队列的长度
		perror("listen");
		exit(1);
	}
	printf("listening...\n");

	struct sockadd_in naddr;
	socklen_t len;
	int nsockfd=accept(sock,(struct sockaddr*)&naddr,&len);
	if(nsockfd==-1){
		perror("accept");
		exit(1);
	}
	printf("client connecting...\n");

	FILE *fp=fopen("./html/index.html","rb");
	char buf[1024];
	memset(buf,0,sizeof(buf));
	fread(buf,1024,1,fp);
	printf("Content is %s\n",buf);
	if(send(nsock,buf,strlen(buf),0)==-1){
		perror("send");
		close(nsockfd);
		exit(1);
	}
	
	close(nsockfd);				//关闭网络连接
	close(sockfd)
	return 0;
}