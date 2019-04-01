#include"WebServer.h"
using namespace std;

int set_nonblock(int sockfd){
	int flags;
	if((flags=fcntl(sockfd,F_GETFL,NULL))<0)
		return -1;
	if(fcntl(sockfd,F_SETFL,flags|O_NONBLOCK)== -1)
		return -1;
	return 0;
}
void *call_back(void *arg){
	if(arg==nullptr)
		return nullptr;
	int *fd=(int *)arg;
	close(*fd);
	return nullptr;
}
int main(int argc,char *argv[]){
	//----------------------声明区----------------------------
	struct sockaddr_in addr,naddr;			//在头文件<netinet/in.h>中
	socklen_t len;
	struct kevent *chlist;		//监听事件
	struct kevent *evlist; 		//触发事件
	struct Req_union *reqs;		//用来处理请求
	threadpool tpool;			//线程池
	memorypool *mpool;			//内存池
	Mng_union *Mymng;			//定时器管理单元
	int reuse=1,kq;				//控制端口复用，kqueue队列
	vector<Heap_entry*>Heap;	//用来管理定时器的堆
	int **fd_arg;				//用来传递多线程参数
	//-------------------------------------------------------

	//---------------------------初始化区-----------------------
	mpool=CreateMemoryPool(MSIZE);
	fd_arg=(int**)GetMemory(MAXEVENT*sizeof(int*),mpool);
	for(int i=0;i<MAXEVENT;i++)
		fd_arg[i]=(int*)GetMemory(sizeof(int),mpool);
	//初始化kevent结构体
	chlist=(struct kevent*)GetMemory(sizeof(struct kevent),mpool);
	evlist=(struct kevent*)GetMemory(sizeof(struct kevent)*MAXEVENT,mpool);

	//初始化定时器管理结构体
	Mymng=(struct Mng_union*)GetMemory(sizeof(struct Mng_union),mpool);
	Mymng->manager=(struct Timer_mng*)GetMemory(sizeof(Timer_mng),mpool);
	Mymng->mutex=(struct my_mutex*)GetMemory(sizeof(my_mutex),mpool);
	reqs=(struct Req_union*)GetMemory(sizeof(struct Req_union)*MAXEVENT,mpool);
	for(int i=0;i<MAXEVENT;i++){
		//reqs[i].request=(http_req*)GetMemory(sizeof(http_req),mpool);
		reqs[i].timer=(Timer*)GetMemory(sizeof(Timer),mpool);
	}
	
	Mymng->mutex->init();
	Mymng->manager->heap=&Heap;			//vector指针操作
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
		//char *buff=(char *)GetMemory(BUFFSIZE*sizeof(char),mpool);	//初始化buff
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
				printf("------------------------------------------------\n");
				if(evlist[i].flags&EV_ERROR){
					printf("EV_ERROR:%s\n",strerror(evlist[i].data));
					exit(EXIT_FAILURE);
				}
				if(evlist[i].ident==sockfd){
					int nsockfd=accept(sockfd,(struct sockaddr*)&naddr,&len);	//呼叫地址
					
					reqs[i].sockfd=nsockfd;
					fd_arg[i]=&reqs[i].sockfd;
					//set_nonblock(nsockfd);	//设置非阻塞
					if(nsockfd==-1){
						perror("accept");
						exit(1);
					}
			////		printf("client connected\n\n");

			////	recv(nsockfd,buff,BUFFSIZE,0);		//recv将接收到的数据放到buff中
			//	send(nsockfd,buff,BUFFSIZE,0);		//向客户端发送buff中的内容

			////	printf("Recive message from client: \n%s\n",buff);

					//---------------线程池操作----------------
			////		reqs[i].request->req_init(nsockfd,buff);
					threadpool_add_task(&tpool,deal_req,fd_arg[i]);	//加入线程池
					//---------------定时器操作--------------
					// int *fd_arg=(int*)malloc(sizeof(int));
			////		fd_arg[i]=&reqs[i].request->sock;
					reqs[i].timer->Start(call_back,fd_arg[i],CBTIME,ONCE);
					Mymng->mutex->lock();					//多线程操作加锁	
					Mymng->manager->AddTimer(reqs[i].timer);
					Mymng->mutex->signal();					//唤醒睡眠的定时器管理线程
					Mymng->mutex->unlock();					//解锁
				//	printf("DEBUG________________SOCKET1:%d\n",nsockfd);
					//deal_req(arg);
				//	free(fd_arg);
				}
				printf("------------------------------------------------\n");
			}
			//	sleep(100);
		}
		//FreeMemory(buff,mpool);
	}

	//--------------内存释放，关闭端口---------------------

	for(int i=0;i<MAXEVENT;i++)
		FreeMemory(fd_arg[i],mpool);
	FreeMemory(fd_arg,mpool);
	FreeMemory(Mymng->manager,mpool);
	FreeMemory(Mymng->mutex,mpool);
	FreeMemory(Mymng,mpool);
	for(int i=0;i<MAXEVENT;i++){
		//FreeMemory(reqs[i].request,mpool);
		FreeMemory(reqs[i].timer,mpool);
	}
	FreeMemory(reqs,mpool);
	FreeMemory(chlist,mpool);				//内存释放
	FreeMemory(evlist,mpool);
	threadpool_destroy(&tpool);	//销毁线程池
	close(kq);
	close(sockfd);				//close用以关闭一般的文件描述符
	ReleaseMemoryPool(mpool);
	return 0;
}