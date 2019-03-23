#ifndef __WEB_SERVER_H_
#define __WEB_SERVER_H_
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
#include<vector>
#include"Deal_req.h"
#include"Mutex/Mutex.h"
#include"File_ope.h"
#include"Thread_pool/Thread_pool.h"
#include"Timer_mng.h"

#define PORT 8848		//端口号
#define BACKLOG 10		//等待队列长度
#define BUFFSIZE 1024
#define MAXEVENT 7
#define CBTIME 10000
struct Req_union{
	http_req *request;
	Timer *timer;
};
struct Mng_union{
	Timer_mng *manager;
	my_mutex *mutex;
};
int set_nonblock(int sockfd);	//设置非阻塞
void *call_back(void *arg);		//关闭套接字的回调函数

#endif
