#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include"Get_html.h"
using namespace std;

int main(){
	int sockfd=socket(AF_INET,SOCK_STREAM,0);//建立套接字
	if(sockfd==-1){
		perror("Socket");
		exit(1);
	}
	struct sockaddr_in addr;			//在头文件<netinet/in.h>中
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);			//转化为网络字节序
	addr.sin_addr.s_addr=INADDR_ANY; 	//初始化为我的IP
	bzero(&(addr.sin_zero),sizeof(addr.sin_zero));	//多余的字节初始为0
	
	//将本地端口和套接字绑定
	if(bind(sockfd,(const struct sockaddr*)&addr,sizeof(addr))==-1){
		perror("bind");
		exit(1);
	}

	if(listen(sockfd,BACKLOG)==-1){	//第二个参数是等待队列的长度
		perror("listen");
		exit(1);
	}
	printf("listening...\n");

	struct sockaddr_in naddr;
	socklen_t len;
	int nsockfd=accept(sockfd,(struct sockaddr*)&naddr,&len);	//呼叫地址
	if(nsockfd==-1){
		perror("accept");
		exit(1);
	}
	printf("client connecting...\n");

	FILE *fp=fopen("/html/index.html","rb");
	char buf[1024];
	memset(buf,0,sizeof(buf));
	fread(buf,1024,1,fp);
	printf("Content is \n%s",buf);
	if(send(nsockfd,buf,strlen(buf),0)==-1){
		perror("send");
		close(nsockfd);
		exit(1);
	}
	
	close(nsockfd);				//关闭网络连接,在unistd.h中
	close(sockfd);				//close用以关闭一般的文件描述符
	return 0;
}