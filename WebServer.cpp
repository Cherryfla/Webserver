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
#include<sys/event.h>
#include<sys/time.h>
#include<fcntl.h>
#include"WebServer.h"
using namespace std;

bool is_get_http(char* request){		//判断是否为GET请求
	return !strncmp(request,"GET",3);
}
char* get_file_name(char* buff){	//从请求中得到文件名
	//printf("DEBUG:-----%s-----\n\n",buff);
	if(buff==NULL||strlen(buff)<4)
		return NULL;
	char* fp=buff+4;
	char* Space=strchr(fp,' ');
	*Space='\0';
	return fp;
}
int get_file_size(char* file_name){	//得到文件大小	
	if(file_name==NULL)
		return -1;
	FILE *fp=fopen(file_name,"r");
	if(!fp) 
		return -1;
	fseek(fp,0,SEEK_END);
	int size=ftell(fp);
	fclose(fp);
	return size;
}
char* get_file_type(char* file_path){	//返回响应报文的文件类型
	if(file_path==NULL)
		return NULL;
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
	if(fp==NULL){
		perror("File");
		return;
	}
	printf("The file path is: %s\n",fp);
	
	FILE* fd=fopen(fp,"rb");

	if(fd==NULL){
		char Error[]="HTTP/1.0 404 Not Found\
		 Content-Type: text/plain 404 not found by Manio";
		send(sock,Error,strlen(Error),0);
	}
	else{
		int file_size=get_file_size(fp);
		//int opened_file=open(fp,O_RDONLY);
		char *memfile=(char *)calloc(file_size,sizeof(char));
		fread(memfile,file_size,1,fd);
		//close(opened_file);		//关闭文件
		
		printf("The file size is:%d\r\n",file_size);
		printf("The context is: \r\n");
		write(1,memfile,file_size);
		printf("\r\n");
		
		char* file_type=get_file_type(fp);
		char buff[BUFFSIZE],FlTp[256],ConLen[256];
		memset(buff,0,sizeof(buff));
		memset(ConLen,0,sizeof(ConLen));
		memset(FlTp,0,sizeof(FlTp));

		strcat(buff,"\r\nHTTP/1.0 200 ok\r\n");		//响应报文的格式
		sprintf(FlTp,"Content-Type: %s;charset=UTF-8\r\n",file_type);
		strcat(buff,FlTp);
	//	strcat(buff,"Content-Type: text/html;charset=UTF-8\r\n");
		sprintf(ConLen,"Content-Length: %d\r\n\r\n",file_size);
		strcat(buff,ConLen);

		send(sock,buff,strlen(buff),0);
		send(sock,memfile,file_size,0);
		printf("Send successfully\r\n\r\n");
		
		free(memfile);
	}
	
}
int main(int argc,char *argv[]){
	struct sockaddr_in addr,naddr;			//在头文件<netinet/in.h>中
	socklen_t len;
	struct kevent *chlist;		//监听事件
	struct kevent *evlist; 		//触发事件
	int kq;						//kqueue队列

	int sockfd=socket(PF_INET,SOCK_STREAM,0);//建立套接字
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

	kq=kqueue();
	if(kq==-1){
		perror("kqueue");
		exit(1);
	}
	//初始化kevent结构体
	chlist=(struct kevent*)malloc(sizeof(struct kevent));
	evlist=(struct kevent*)malloc(sizeof(struct kevent)*MAXEVENT);

	EV_SET(chlist,sockfd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,0);	//注册事件
	while(true){
		char* buff=(char *)calloc(BUFFSIZE,sizeof(char));	//初始化buff
		int nev=kevent(kq,chlist,1,evlist,MAXEVENT,NULL);	//无限阻塞
		if(nev<0){
			perror("kevent");
		}
		else{
			printf("------------------------------------------------\n");
			printf("The num of req is:%d\n",nev);
			if(evlist[0].flags&EV_EOF){//读取socket关闭指示
				exit(EXIT_FAILURE);
			}
			for(int i=0;i<nev;i++){
				if(evlist[i].flags&EV_ERROR){
					printf("EV_ERROR:%s\n",strerror(evlist[i].data));
					exit(EXIT_FAILURE);
				}
				if(evlist[i].ident==sockfd){
					int nsockfd=accept(sockfd,(struct sockaddr*)&naddr,&len);	//呼叫地址
					if(nsockfd==-1){
						perror("accept");
						exit(1);
					}
					printf("client connected\n\n");

					recv(nsockfd,buff,BUFFSIZE,0);		//recv将接收到的数据放到buff中
				//	send(nsockfd,buff,BUFFSIZE,0);		//向客户端发送buff中的内容

					printf("Recive message from client: \n%s\n",buff);

					if(is_get_http(buff)){
						printf("Got request.\n");
						deal_http(nsockfd,buff);
					}
					close(nsockfd);				//关闭网络连接,在unistd.h中
				}
			}
			printf("------------------------------------------------\n");
			//	sleep(100);
		}
		free(buff);
	}

	free(chlist);				//内存释放
	free(evlist);
	close(kq);
	close(sockfd);				//close用以关闭一般的文件描述符
	return 0;
}