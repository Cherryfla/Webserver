#include"Common.h"

my_mutex Req_union::ulock=union_lock;
int set_nonblock(int sockfd){
	int flags;
	if((flags=fcntl(sockfd,F_GETFL,NULL))<0)
		return -1;
	if(fcntl(sockfd,F_SETFL,flags|O_NONBLOCK)== -1)
		return -1;
	return 0;
}
void *Timers_det(void *arg){
    if(arg==nullptr)
        return nullptr;
    Mng_union *mng_union=(Mng_union *)arg;
    if(mng_union->mutex==nullptr||mng_union->manager==nullptr)
        return nullptr;
    while(true){
        mng_union->mutex->lock();
        if(mng_union->manager->GetSize()==0)
            mng_union->mutex->wait();
        mng_union->manager->DetectTimers();
        mng_union->mutex->unlock();
    }
    return nullptr;
}
void *call_back(void *arg){
	if(arg==nullptr)
		return nullptr;
	int *fd=(int *)arg;
	close(*fd);
	return nullptr;
}
void* deal_req(void* arg){
	Req_union *Got_arg=static_cast<Req_union*>(arg);
	char *buff=static_cast<char*>(get_memory(BUFFSIZE*sizeof(char),mpool));

	recv(Got_arg->sockfd,buff,BUFFSIZE,0);
    
	http_req *Got_req=new http_req(Got_arg->sockfd,buff);

	printf("------------------------------------------------\n");
	printf("Recive message from client: \n%s\n",buff);
    printf("------------------------------------------------\n");   


	Got_req->req_break();
	Got_req->reqline_analyse();
    //printf("%s\n",Got_req->get_file_path());

	if(!strcmp(Got_req->get_method(),"POST")){
		Got_req->deal_post();
	}
	else if(!strcmp(Got_req->get_method(),"GET")){
		Got_req->deal_get();
	}
	struct stat buf;
	int ret=stat(Got_req->get_file_path(),&buf);
	if(ret==-1){
		if(errno==ENOENT)			//文件不存在
			Got_req->http_404(buf);
		else if(errno==EACCES)		//访问受限，linux中为EACCESS，unix中为EACCES
			Got_req->http_403(buf);
	}
	else{
		FILE *fd=fopen(Got_req->get_file_path(),"rb");
		int file_size=get_file_size(Got_req->get_file_path());
		//int opened_file=open(fp,O_RDONLY);
		char *memfile=(char *)calloc(file_size,sizeof(char));
		fread(memfile,file_size,1,fd);
		//close(opened_file);		//关闭文件
		printf("The file size is:%d\r\n",file_size);
		printf("The context is: \r\n");
		write(1,memfile,file_size);
		
        printf("\r\n");
		
		char buff[1024],FlTp[256],ConLen[256];
		memset(buff,0,sizeof(buff));
		memset(ConLen,0,sizeof(ConLen));
		memset(FlTp,0,sizeof(FlTp));

		strcat(buff,"\r\nHTTP/1.0 200 ok\r\n");		//响应报文的格式
		strcat(buff,KEEPALIVE?"Connection: keep-alive\r\n":"Connection: keep-alive\r\n");
		sprintf(FlTp,"Content-Type: %s;charset=UTF-8\r\n",Got_req->get_file_type());
		strcat(buff,FlTp);
	//	strcat(buff,"Content-Type: text/html;charset=UTF-8\r\n");
		sprintf(ConLen,"Content-Length: %d\r\n\r\n",file_size);
		strcat(buff,ConLen);

		send(Got_req->get_sock(),buff,strlen(buff),0);
		send(Got_req->get_sock(),memfile,file_size,0);
		printf("Send successfully\r\n\r\n");
		
//		close(Got_req->sock);
		free(memfile);
	}

	printf("client connected\n\n");
	free_memory(buff,mpool);
	delete Got_req;
	return nullptr;
}
