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
#include"Deal_req.h"
#include"File_ope.h"
using namespace std;

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
		char *buff=(char *)calloc(BUFFSIZE,sizeof(char));	//初始化buff
		int nev=kevent(kq,chlist,1,evlist,MAXEVENT,NULL);	//无限阻塞
		if(nev<0){
			perror("kevent");
		}
		else{
			printf("------------------------------------------------\n");
			printf("The num of req is:%d\n",nev);
			if(evlist[0].flags&EV_EOF){		//读取socket关闭指示
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

					if(buff!=NULL){
						printf("Got request.\n");
						http_req new_req;
						new_req.deal_req(nsockfd,buff);
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