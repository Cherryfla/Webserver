#include<cstdio>
#include<stdio.h>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include"Get_html.h"
using namespace std;

bool is_get_http(char* request){		//判断是否为GET请求
	return !strncmp(request,"GET",3);
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
	fseek(fp,0,SEEK_END);
	int size=ftell(fp);
	fclose(fp);
	return size;
}
char* get_file_type(char* file_path){	//返回响应报文的文件类型
	char* tmp=strrchr(file_path,'.');
	static char file_type[256];
	if(tmp==NULL)
		strcpy(file_type,"text/plain");
	else
		tmp++;
	if(!strcmp(tmp,"html")||!strcmp(tmp,"htm"))
		strcpy(file_type,"text/html");
	else if(!strcmp(tmp,"css"))
		strcpy(file_type,"text/css");
	else if(!strcmp(tmp,"gif"))
		strcpy(file_type,"image/gif");
	else if(!strcmp(tmp,"jpeg")||!strcmp(tmp,"jpg"))
		strcpy(file_type,"image/jpeg");
	else if(!strcmp(tmp,"png"))
		strcpy(file_type,"image/png");
	else
		strcpy(file_type,"text/plain");
	return file_type;
}
void deal_http(int sock,char* request){
	char* fp=get_file_name(request);
	printf("file path is :%s\n",fp);
	
	FILE* fd=fopen(fp,"rb");

	if(fd==NULL){
		char Error[]="HTTP/1.0 404 Not Found\
		 Content-Type: text/plain 404 not found by Manio";
		send(sock,Error,strlen(Error),0);
	}
	else{
		int file_size=get_file_size(fp);
		int opened_file=open(fp,O_RDONLY);
		void* memfile=mmap(0,file_size,PROT_READ,MAP_PRIVATE,opened_file,0);
		//存储映射，防止因文件过大爆栈
		close(opened_file);		//关闭文件
		
		printf("The file size is:%d\r\n",file_size);
		printf("The context is \r\n");
		write(1,memfile,file_size);
		printf("\r\n");
		
		char* file_type=get_file_type(fp);
		char buff[BUFFSIZE],FlTp[256],ConLen[256];
		memset(buff,0,sizeof(buff));
		memset(ConLen,0,sizeof(ConLen));
		memset(FlTp,0,sizeof(FlTp));

		strcat(buff,"HTTP1.0 200 ok\r\n");		//响应报文的格式
		sprintf(FlTp,"Content-Type: %scharset=UTF-8\r\n",file_type);
		strcat(buff,FlTp);
	//	strcat(buff,"Content-Type: text/html;charset=UTF-8\r\n");
		sprintf(ConLen,"Content-Length: %d\r\n\r\n",file_size);
		strcat(buff,ConLen);

		send(sock,buff,strlen(buff),0);
		send(sock,memfile,file_size,0);
		printf("Send successfully\n");

		munmap(memfile,file_size);
	}
}
int main(int argc,char *argv[]){
	struct sockaddr_in addr,naddr;			//在头文件<netinet/in.h>中
	socklen_t len;
	char buff[BUFFSIZE];
	
	memset(buff,0,sizeof(buff));
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
//	send(nsockfd,buff,BUFFSIZE-1,0);		//向客户端发送buff中的内容

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