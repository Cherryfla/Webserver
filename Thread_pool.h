#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_
#include"Mutex.h"
#define ul unsigned long
#define WAIT_TIME 2	//空闲任务等待时间
//任务对象
struct task{
	void *(*run)(void *args);
	void *arg;
	task *next;
};

//线程池结构体
struct threadpool{
	my_mutex ready;
	task *first,*last;	//任务队列第一个和随后一个任务
	int sum,idle;		//总的线程数和空闲线程数
	int quit;			//退出标志
	int max_threads;	//最大线程数
};
//初始化线程
void threadpool_init(threadpool *pool,int max_threads);
//往线程池中添加任务
void threadpool_add_task(threadpool *pool,void *(*run)(void *arg),void *arg);
//执行任务
void *thread_routine(void *arg);
//摧毁线程池
void threadpool_destroy(threadpool *pool);
#endif