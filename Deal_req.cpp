#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<errno.h>
#include"Deal_req.h"
#include"File_ope.h"
using namespace std;

int http_req::req_init(int n_sock,char *n_req){
	sock=n_sock;
	int len=strlen(n_req);
	req=(char *)calloc(len,sizeof(char));
	strcpy(req,n_req);
	return 0;
}
int http_req::req_break(){
	char *tmp=req;
	if(tmp==nullptr)
		return -1;
	req_line=tmp;
	tmp=strchr(tmp,'\r');
	if(tmp==nullptr)
		return -1;
	*tmp='\0';
	tmp++;
	*tmp='\0';
	tmp++;
	req_head=tmp;
	int cnt=0;
	for(;cnt!=2&&*tmp!='\0';){
		if(*tmp=='\r'){
			cnt++;
		}
		else if(*tmp!='\n')
			cnt=0;
		tmp++;
	}
	*(tmp-1)='\0';
	*tmp='\0';
	tmp++;
	req_body=tmp;
	return 0;
}
int http_req::reqline_analyse(){
	if(req_line==nullptr)
		return -1;
	char *tmp=req_line;
	method=tmp;
	tmp=strchr(tmp,' ');
	if(tmp==nullptr)
		return -1;
	for(int i=0;i<2;i++){
		*tmp='\0';
		tmp++;
	}
	url=tmp;
	tmp=strchr(tmp,' ');
	if(tmp==nullptr)
		return -1;
	*tmp='\0';
	tmp++;
	version=tmp;
	return 0;
}
int http_req::deal_get(){
	char *p=strchr(url,'?');
	if(p!=nullptr){
		argv=p+1;		//获取get参数
		*p='\0';
	}
	strcpy(file_path,url);
	if(file_path[0]=='\0')
		return -1;
	char *tmp=strrchr(file_path,'.');
	memset(file_type,0,sizeof(file_type));
	if(tmp==nullptr)
		strcpy(file_type,"text/plain");
	else
		tmp++;
	if(!strcmp(tmp,"html")||!strcmp(tmp,"htm"))
		strcpy(file_type,"text/html");
	else if(!strcmp(tmp,"css"))
		strcpy(file_type,"text/css");
	else if(!strcmp(tmp,"gif"))
		strcpy(file_type,"image/gif");
	else if(!strcmp(tmp,"jpeg")||!strcmp(tmp,"jpg"))
		strcpy(file_type,"image/jpeg");
	else if(!strcmp(tmp,"png"))
		strcpy(file_type,"image/png");
	else
		strcpy(file_type,"text/plain");
	return 0;
}
int http_req::deal_post(){
	strcpy(file_path,url);
	if(file_path[0]=='\0')
		return -1;
	char *tmp=strrchr(file_path,'.');
	if(tmp==nullptr)
		strcpy(file_type,"text/plain");
	else
		tmp++;
	memset(file_type,0,sizeof(file_type));
	if(!strcmp(tmp,"html")||!strcmp(tmp,"htm"))
		strcpy(file_type,"text/html");
	else if(!strcmp(tmp,"css"))
		strcpy(file_type,"text/css");
	else if(!strcmp(tmp,"gif"))
		strcpy(file_type,"image/gif");
	else if(!strcmp(tmp,"jpeg")||!strcmp(tmp,"jpg"))
		strcpy(file_type,"image/jpeg");
	else if(!strcmp(tmp,"png"))
		strcpy(file_type,"image/png");
	else if(!strcmp(tmp,"ico"))
		strcpy(file_type,"image/x-icon")
	else
		strcpy(file_type,"text/plain");
	return 0;
}
int http_req::http_404(struct stat buf){
	char *Error;
	Error=(char *)calloc(BUFFSIZE,sizeof(char));
	sprintf(Error,"HTTP/1.1 404 NOT FOUND\r\n\
	Connection: close\r\ncontent-length:%lld\r\n\r\n",0ll);
	int res=send(sock,Error,strlen(Error),0);
	free(Error);
	return res;
}
int http_req::http_403(struct stat buf){
	char *Error;
	Error=(char *)calloc(BUFFSIZE,sizeof(char));
	sprintf(Error,"HTTP/1.1 403 FORBIDDEN\r\n\
	Connection: close\r\ncontent-length:%lld\r\n\r\n",0ll);
	int res=send(sock,Error,strlen(Error),0);
	free(Error);
	return res;
}

void* deal_req(void* arg){
	int* Got_arg=static_cast<int*>(arg);
	char *buff=new char [BUFFSIZE];
	http_req *Got_req=new http_req;

	recv(*Got_arg,buff,BUFFSIZE,0);
	printf("Recive message from client: \n%s\n",buff);

	Got_req->req_init(*Got_arg,buff);
//	printf("DEBUG___________socket2: %d\n",Got_req->sock);
	Got_req->req_break();
	Got_req->reqline_analyse();
	if(!strcmp(Got_req->method,"POST")){
		Got_req->deal_post();
	}
	else if(!strcmp(Got_req->method,"GET")){
		Got_req->deal_get();
	}
	struct stat buf;
	int ret=stat(Got_req->file_path,&buf);
	if(ret==-1){
		if(errno==ENOENT)			//文件不存在
			Got_req->http_404(buf);
		else if(errno==EACCES)		//访问受限，linux中为EACCESS，unix中为EACCES
			Got_req->http_403(buf);
	}
	else{
		FILE *fd=fopen(Got_req->file_path,"rb");
		int file_size=get_file_size(Got_req->file_path);
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
		sprintf(FlTp,"Content-Type: %s;charset=UTF-8\r\n",Got_req->file_type);
		strcat(buff,FlTp);
	//	strcat(buff,"Content-Type: text/html;charset=UTF-8\r\n");
		sprintf(ConLen,"Content-Length: %d\r\n\r\n",file_size);
		strcat(buff,ConLen);

		send(Got_req->sock,buff,strlen(buff),0);
		send(Got_req->sock,memfile,file_size,0);
		printf("Send successfully\r\n\r\n");
		
//		close(Got_req->sock);
		free(memfile);
	}

	printf("client connected\n\n");
	delete [] buff;
	delete Got_req;
	return nullptr;
}