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


int http_req::req_break(char *req){
	char *tmp=req;
	if(tmp==NULL)
		return -1;
	req_line=tmp;
	tmp=strchr(tmp,'\r');
	if(tmp==NULL)
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
	if(req_line==NULL)
		return -1;
	char *tmp=req_line;
	method=tmp;
	tmp=strchr(tmp,' ');
	if(tmp==NULL)
		return -1;
	*tmp='\0';
	tmp++;
	url=tmp;
	tmp=strchr(tmp,' ');
	if(tmp==NULL)
		return -1;
	*tmp='\0';
	tmp++;
	version=tmp;
	return 0;
}
int http_req::deal_get(){
	char *p=strchr(url,'?');
	if(p!=NULL){
		argv=p+1;		//获取get参数
		*p='\0';
	}
	strcpy(file_path,url);
	if(file_path[0]=='\0')
		return -1;
	char *tmp=strrchr(file_path,'.');
	memset(file_type,0,sizeof(file_type));
	if(tmp==NULL)
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
	if(tmp==NULL)
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
	else
		strcpy(file_type,"text/plain");
	return 0;
}
int http_req::http_404(int sock,struct stat buf){
	char *Error;
	Error=(char *)calloc(BUFFSIZE,sizeof(char));
	sprintf(Error,"HTTP/1.1 404 NOT_FOUND\r\n\
	Connection: close\r\ncontent-length:%lld\r\n\r\n",buf.st_size);
	int res=send(sock,Error,strlen(Error),0);
	free(Error);
	return res;
}
int http_req::http_403(int sock,struct stat buf){
	char *Error;
	Error=(char *)calloc(BUFFSIZE,sizeof(char));
	sprintf(Error,"HTTP/1.1 403 FORBIDDEN\r\n\
	Connection: close\r\ncontent-length:%lld\r\n\r\n",buf.st_size);
	int res=send(sock,Error,strlen(Error),0);
	free(Error);
	return res;
}
int http_req::deal_req(int sock,char *req){
	req_break(req);
	reqline_analyse();
	if(!strcmp(method,"POST")){
		deal_post();
	}
	else if(!strcmp(method,"GET")){
		deal_get();
	}
	struct stat buf;
	int ret=stat(file_path,&buf);
	if(ret==-1){
		if(errno==ENOENT)			//文件不存在
			http_404(sock,buf);
		else if(errno==EACCES)		//访问受限，linux中为EACCESS，unix中为EACCES
			http_403(sock,buf);
	}
	else{
		FILE *fd=fopen(file_path,"rb");
		int file_size=get_file_size(file_path);
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
		sprintf(FlTp,"Content-Type: %s;charset=UTF-8\r\n",file_type);
		strcat(buff,FlTp);
	//	strcat(buff,"Content-Type: text/html;charset=UTF-8\r\n");
		sprintf(ConLen,"Content-Length: %d\r\n\r\n",file_size);
		strcat(buff,ConLen);

		send(sock,buff,strlen(buff),0);
		send(sock,memfile,file_size,0);
		printf("Send successfully\r\n\r\n");
		
		free(memfile);
	}
	return 0;
}