#include<cstdio>
#include<stdio.h>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include"Get_html.h"
using namespace std;

bool is_get_http(char* buff){	//判断是否为GET请求
	return !strncmp(buff,"GET",3);
}

char* get_file_name(char* buff){	//从请求中得到文件名
	char* fp=buff+4;
	char* Space=strchr(fp,' ');
	*Space='\0';
	return fp;
}
int get_file_size(char* file_name){	//得到文件大小	
    FILE *fp=fopen(file_name,"r");
    if(!fp) 
    	return -1;
    fseek(fp,0L,SEEK_END);
    int size=ftell(fp);
    fclose(fp);
    return size;
}
void deal_http(int sock,char* buff){
	char* fp=get_file_name(buff);
	printf("file path is :%s\n",fp);
	
	FILE* fd=fopen(fp,"rb");

	if(fd==NULL){
		char Error[]="Error:404\n";
		send(sock,Error,strlen(Error),0);
	}
	else{
		int file_size=get_file_size(fp);
		char tmp[BUFFSIZE];
		memset(tmp,0,sizeof(tmp));
		fread(tmp,BUFFSIZE,1,fd);
		fclose(fd);		//关闭文件
		
		printf("The file size is:%d\r\n",file_size);
		printf("The context is \n%s\n",tmp);
		if(~send(sock,tmp,strlen(tmp),0)){
			printf("Send successfully\n");
		}
		else{
			perror("send");
		}
	}
}
int main(int argc,char *argv[]){
	struct sockaddr_in addr,naddr;			//在头文件<netinet/in.h>中
	socklen_t len;
	char buff[BUFFSIZE];
	int sockfd=socket(AF_INET,SOCK_STREAM,0);//建立套接字
	if(sockfd==-1){
		perror("Socket");
		exit(1);
	}

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

	int nsockfd=accept(sockfd,(struct sockaddr*)&naddr,&len);	//呼叫地址
	if(nsockfd==-1){
		perror("accept");
		exit(1);
	}
	printf("client connected\n");

	recv(nsockfd,buff,BUFFSIZE-1,0);		//recv将接收到的数据放到buff中
	send(nsockfd,buff,BUFFSIZE-1,0);		//向客户端发送buff中的内容

	printf("Recive message from client: %s\n",buff);

	if(is_get_http(buff)){
		printf("Got request.\n");
		deal_http(nsockfd,buff);
	}

//	sleep(100);
	close(nsockfd);				//关闭网络连接,在unistd.h中
	close(sockfd);				//close用以关闭一般的文件描述符
	return 0;
}