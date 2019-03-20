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
	threadpool pool;			//线程池
	Mng_union *Mymng;			//定时器管理单元
	int reuse=1,kq;				//控制端口复用，kqueue队列
	vector<Heap_entry*>Heap;	//用来管理定时器的堆
	//-------------------------------------------------------
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
	//---------------------------初始化区-----------------------
	//初始化kevent结构体
	chlist=(struct kevent*)malloc(sizeof(struct kevent));
	evlist=(struct kevent*)malloc(sizeof(struct kevent)*MAXEVENT);
	//初始化定时器管理结构体
	Mymng=(struct Mng_union*)malloc(sizeof(struct Mng_union));
	Mymng->manager=(struct Timer_mng*)malloc(sizeof(Timer_mng));
	Mymng->mutex=(struct my_mutex*)malloc(sizeof(my_mutex));
	Mymng->manager->heap=&Heap;
	//初始化请求结构体
	reqs=(struct Req_union*)malloc(sizeof(struct Req_union)*MAXEVENT);
	for(int i=0;i<MAXEVENT;i++){
		reqs[i].request=(http_req*)malloc(sizeof(http_req));
		reqs[i].timer=(Timer*)malloc(sizeof(Timer));
	}
    //初始化线程池线程数为最大能处理请求
    threadpool_init(&pool, MAXEVENT+1);
    //--------------------------------------------------------
	EV_SET(chlist,sockfd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,0);	//注册事件
	threadpool_add_task(&pool,Timers_det,Mymng);				//开启一个线程用来管理定时器
	while(true){
		char *buff=(char *)calloc(BUFFSIZE,sizeof(char));	//初始化buff
		int nev=kevent(kq,chlist,1,evlist,MAXEVENT,nullptr);	//无限阻塞
		if(nev<0){
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

					//set_nonblock(nsockfd);	//设置非阻塞
					if(nsockfd==-1){
						perror("accept");
						exit(1);
					}
					printf("client connected\n\n");

					recv(nsockfd,buff,BUFFSIZE,0);		//recv将接收到的数据放到buff中
				//	send(nsockfd,buff,BUFFSIZE,0);		//向客户端发送buff中的内容

					printf("Recive message from client: \n%s\n",buff);

					if(buff!=nullptr){
						printf("--------Got request.-------\n");
						//---------------线程池操作----------------
						reqs[i].request->req_init(nsockfd,buff);
						threadpool_add_task(&pool,deal_req,reqs[i].request);	//加入线程池
						//---------------定时器操作--------------
						int *fd_arg=(int*)malloc(sizeof(int));
						*fd_arg=reqs[i].request->sock;
						reqs[i].timer->Start(call_back,fd_arg,CBTIME,ONCE);
						Mymng->mutex->lock();

						// reqs[i].timer->heapIndex=Mymng->manager->heap->size();
						// Heap_entry *entry=(Heap_entry*)malloc(sizeof(Heap_entry));
					 //    entry->time=reqs[i].timer->expires;
					 //    entry->timer=reqs[i].timer;
						// Mymng->manager->heap->push_back(entry);
						// printf("11111\n");
						
						Mymng->manager->AddTimer(reqs[i].timer);
						Mymng->mutex->signal();
						Mymng->mutex->unlock();
					//	printf("DEBUG________________SOCKET1:%d\n",nsockfd);
						//deal_req(arg);
					//	free(fd_arg);
					}
				}
				printf("------------------------------------------------\n");
			}
			
			//	sleep(100);
		}
		free(buff);
	}

	//--------------内存释放，关闭端口---------------------
	free(Mymng->manager);
	free(Mymng->mutex);
	free(Mymng);
	for(int i=0;i<MAXEVENT;i++){
		free(reqs[i].request);
		free(reqs[i].timer);
	}
	free(reqs);
	threadpool_destroy(&pool);	//销毁线程池
	free(chlist);				//内存释放
	free(evlist);
	close(kq);
	close(sockfd);				//close用以关闭一般的文件描述符
	return 0;
}