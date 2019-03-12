#include<cstdio>
#include<cstring>
#include<time.h>
#include<cstdlib>
#include<errno.h>
#include"Thread_pool.h"

void threadpool_init(threadpool *pool,int max_threads){
	pool->ready.init();
	pool->first=pool->last=NULL;
	pool->sum=pool->idle=0;
	pool->max_threads=max_threads;
	pool->quit=0;
}
void threadpool_add_task(threadpool *pool,void *(*run)(void *arg),void *arg){
	task *newtask=(task *)malloc(sizeof(task));
	newtask->run=run,newtask->arg=arg,newtask->next=NULL;	//创建新任务

	pool->ready.lock();//线程池可能被多个线程操作，因此需要加锁

	if(pool->first==NULL){	//第一个任务加入
		pool->first=newtask;
	}
	else{
		pool->last->next=newtask;
	}
	pool->last=newtask;

	if(pool->idle){
		pool->ready.signal();
	}
	else if(pool->sum<pool->max_threads){
		pthread_t tid;
		pthread_create(&tid,NULL,thread_routine,pool);
		pool->sum++;
	}
	pool->ready.unlock();
}
void *thread_routine(void *arg){
	printf("thread %lu is starting.\n",(ul)pthread_self());
	threadpool *pool=(threadpool *)arg;

	while(1){
		pool->ready.lock();	//加锁防止多线程出错

		//任务列表不为空，肯定要先执行等待队列中的任务
		if(pool->first!=NULL){
			task *t=pool->first;
			pool->first=t->next;
			pool->ready.unlock();	//任务执行需要时间，所以先解锁让其他线程有机会访问
			t->run(t->arg);			//执行任务
			free(t);				//释放内存
			pool->ready.lock();		//任务执行完成，重新尝试获取控制权
		}
		//任务列表为空了，又接受到了线程退出通知，只能退出线程
		if(pool->first==NULL&&pool->quit){
			pool->sum--;			//当前线程从线程池中退出，线程总数减-
			//线程池中以及没有线程了，唤醒主线程
			if(pool->sum==0){
				pool->ready.signal();
			}
			pool->ready.unlock();	//感觉这里有点问题
			break;
		}
		//此时既无任务可执行，又没有接到线程销毁的通知，所以只能轮询
		int timeout=0;				//用来判断是否超时的标志
		pool->idle++;				//当前进程空闲，空闲进程++
		while(pool->first==NULL&&!pool->quit){
			printf("thread %lu is waiting\n",(ul)pthread_self());
			struct timespec now;
			clock_gettime(CLOCK_REALTIME,&now);//获取实时时间
			now.tv_sec+=WAIT_TIME;	//加上空闲任务等待时间
			//有限等待，等待其他任务唤醒
			int status=pool->ready.timedwait(&now);
			if(status==ETIMEDOUT){
				printf("thread %lu wait timed out.\n",(ul)pthread_self());
				timeout=1;			//设置超时标志
				break;
			}
		}
		pool->idle--;
		//超时无任务执行,线程退出
		if(timeout){
			pool->sum--;
			pool->ready.unlock();
			break;
		}
		pool->ready.unlock();
	}
	printf("thread %lu is exiting.\n",(ul)pthread_self());
	return NULL;	
}
void threadpool_destroy(threadpool *pool){
	if(pool->quit)
		return ;
	pool->ready.lock();
	pool->quit=1;
	//线程池中还有任务，不能马上结束
	if(pool->sum>0){
		if(pool->idle>0){
			pool->ready.broadcast();	//唤醒所有阻塞线程
		}
		//以轮询的方式等待线程结束
		while(pool->sum){
			pool->ready.wait();
		}
	}
	pool->ready.unlock();
	pool->ready.destroy();
}