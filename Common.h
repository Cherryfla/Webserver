#ifndef __COMMON_H_
#define __COMMON_H_
#include<cstdio>
#include<iostream>
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
#include"File_ope.h"
#include"Timer/Timer_mng.h"
#include"Mutex/Mutex.h"
#include"Thread_pool/Thread_pool.h"
#include"Memory_pool/Memory_pool.h"

#define PORT 8848		//端口号
#define BACKLOG 10		//等待队列长度
#define BUFFSIZE 1024
#define MAXEVENT 7
#define CBTIME 10000

extern memorypool *mpool;
extern my_mutex union_lock;

const size_t MSIZE=128*1024*1024;
struct Req_union{
    static my_mutex ulock;
	int sockfd;
	Timer *timer;
};

struct Mng_union{
	Timer_mng *manager;
	my_mutex *mutex;
};
int set_nonblock(int sockfd);	//设置非阻塞
void *Timers_det(void *arg);    //循环检测定时器
void *call_back(void *arg);		//关闭套接字的回调函数
void *deal_req(void *arg);	    //处理请求的主要函数
#endif
