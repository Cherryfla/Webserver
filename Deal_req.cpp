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

http_req::~http_req(){
    delete [] req;
}
http_req::http_req(int n_sock,char *n_req){
	sock=n_sock;
	int len=strlen(n_req);
	req=new char [len]();
	strcpy(req,n_req);
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
		strcpy(file_type,"image/x-icon");
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
const char* http_req::get_method(){
    return this->method;
}
const char* http_req::get_file_path(){
    return this->file_path;
}
const char* http_req::get_file_type(){
    return this->file_type;
}
const int& http_req::get_sock(){
    return this->sock;
}


