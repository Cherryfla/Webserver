#ifndef __MUTEX_H_
#define __MUTEX_H_
#include<pthread.h>

struct my_mutex{
	pthread_mutex_t mmutex;
	pthread_cond_t mcond;
	int init();		//初始化
	int destroy();	//释放
	int lock();		//加锁
	int unlock();	//解锁
	int wait(); 	//信号量P操作
	int timedwait(const struct timespec *tm);//有限时间等待
	int signal();	//信号量V操作
	int broadcast(); //唤醒所有线程
};

#endif