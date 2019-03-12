#include<cstdio>
#include<cstring>
#include"Mutex.h"

int my_mutex::init(){
	int res;
	res=pthread_mutex_init(&mmutex,NULL);
	if(res)
		return res;
	res=pthread_cond_init(&mcond,NULL);
	return res;
}
int my_mutex::destroy(){
	int res;
	res=pthread_mutex_destroy(&mmutex);
	if(res)
		return res;
	res=pthread_cond_destroy(&mcond);
	return res;
}
int my_mutex::lock(){
	return pthread_mutex_lock(&mmutex);
}
int my_mutex::unlock(){
	return pthread_mutex_unlock(&mmutex);
}
int my_mutex::wait(){
	return pthread_cond_wait(&mcond,&mmutex);
}
int my_mutex::timedwait(const struct timespec *tm){
	return pthread_cond_timedwait(&mcond,&mmutex,tm);
}
int my_mutex::signal(){
	return pthread_cond_signal(&mcond);
}
int my_mutex::broadcast(){
	return pthread_cond_broadcast(&mcond);
}
