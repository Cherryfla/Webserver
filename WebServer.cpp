#include"Common.h"
using namespace std;

memorypool *mpool;      //内存池
my_mutex union_lock;    //用于定时器的锁
int main(int argc,char *argv[]){
	//----------------------声明区----------------------------
	struct sockaddr_in addr,naddr;			//在头文件<netinet/in.h>中
	socklen_t len;
	struct kevent *chlist;		//监听事件
	struct kevent *evlist; 		//触发事件
	struct Req_union *reqs;		//用来处理请求
	threadpool tpool;			//线程池
	Mng_union *Mymng;			//定时器管理单元
	int reuse=1,kq;				//控制端口复用，kqueue队列
	vector<Heap_entry*>Heap;	//用来管理定时器的堆
//	int **fd_arg;				//用来传递多线程参数
	//-------------------------------------------------------

	//---------------------------初始化区-----------------------
//    union_lock.init();
	mpool=CreateMemoryPool(MSIZE);
//	fd_arg=static_cast<int**>(get_memory(MAXEVENT*sizeof(int*),mpool));
//	for(int i=0;i<MAXEVENT;i++)
//		fd_arg[i]=static_cast<int*>(get_memory(sizeof(int),mpool));
	//初始化kevent结构体
	chlist=static_cast<struct kevent*>(get_memory(sizeof(struct kevent),mpool));
	evlist=static_cast<struct kevent*>(get_memory(sizeof(struct kevent)*MAXEVENT,mpool));

	//初始化定时器管理结构体
	Mymng=static_cast<struct Mng_union*>(get_memory(sizeof(struct Mng_union),mpool));
	Mymng->manager=static_cast<struct Timer_mng*>(get_memory(sizeof(Timer_mng),mpool));
	Mymng->mutex=static_cast<struct my_mutex*>(get_memory(sizeof(my_mutex),mpool));
	reqs=static_cast<struct Req_union*>(get_memory(sizeof(struct Req_union)*MAXEVENT,mpool));
	for(int i=0;i<MAXEVENT;i++){
		//reqs[i].request=(http_req*)get_memory(sizeof(http_req),mpool);
		reqs[i].timer=static_cast<Timer*>(get_memory(sizeof(Timer),mpool));
	}
	
	Mymng->mutex->init();
	Mymng->manager->SetHeap(Heap);			//vector指针操作
    threadpool_init(&tpool, MAXEVENT+1);
    //--------------------------------------------------------
	int sockfd=socket(PF_INET,SOCK_STREAM,0);//建立套接字
	if(sockfd==-1){
		perror("Socket");
		exit(1);
	}
	//bind: Address already in use,端口复用
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0){
		perror("setsockopt");
		exit(1);
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT);			//转化为网络字节序
	addr.sin_addr.s_addr=INADDR_ANY; 	//初始化为我的IP
	bzero(&(addr.sin_zero),sizeof(addr.sin_zero));	//多余的字节初始为0
	
	//将本地端口和套接字绑定
	::bind(sockfd,(const struct sockaddr*)&addr,sizeof(addr));
	//bind(sockfd,(const struct sockaddr*)&addr,sizeof(addr),1);
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

	EV_SET(chlist,sockfd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,0);	//注册事件
	threadpool_add_task(&tpool,Timers_det,Mymng);				//开启一个线程用来管理定时器
	while(true){
		int nev=kevent(kq,chlist,1,evlist,MAXEVENT,nullptr);	//无限阻塞
		if(nev<=0){
			perror("kevent");
		}
		else if(nev>MAXEVENT){
			printf("Too many requests\n");
			exit(1);
		}
		else{
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

                	printf("client connected\n\n");

					reqs[i].sockfd=nsockfd;
				//	fd_arg[i]=&reqs[i].sockfd;
					//set_nonblock(nsockfd);	//设置非阻塞
					if(nsockfd==-1){
						perror("accept");
						exit(1);
					}
					//---------------线程池操作----------------
					threadpool_add_task(&tpool,deal_req,&reqs[i]);	//加入线程池
					//---------------定时器操作--------------
					reqs[i].timer->Start(call_back,&(reqs[i].sockfd),CBTIME,ONCE);
					Mymng->mutex->lock();					//多线程操作加锁	
					Mymng->manager->AddTimer(reqs[i].timer);
					Mymng->mutex->signal();					//唤醒睡眠的定时器管理线程
					Mymng->mutex->unlock();					//解锁

				}
			}
			//	sleep(100);
		}
	}

	//--------------内存释放，关闭端口---------------------

//	for(int i=0;i<MAXEVENT;i++)
	// 	free_memory(fd_arg[i],mpool);
	// free_memory(fd_arg,mpool);
	free_memory(Mymng->manager,mpool);
	free_memory(Mymng->mutex,mpool);
	free_memory(Mymng,mpool);
	for(int i=0;i<MAXEVENT;i++){
		//free_memory(reqs[i].request,mpool);
		free_memory(reqs[i].timer,mpool);
	}
	free_memory(reqs,mpool);
	free_memory(chlist,mpool);				//内存释放
	free_memory(evlist,mpool);
	threadpool_destroy(&tpool);	//销毁线程池
	close(kq);
	close(sockfd);				//close用以关闭一般的文件描述符
	ReleaseMemoryPool(mpool);
	return 0;
}