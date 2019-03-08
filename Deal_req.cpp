#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include"Deal.req.h"
using namespace std;
http_req::int req_break(char *req){
	char *tmp=req;
	if(tmp==NULL)
		return -1;
	req_line=tmp;
	while(*tmp!='\r'){
		tmp++;
	}
	*tmp='\0';
	tmp++;
	tmp='\0';
	tmp++;
	req_head=tmp;
	int cnt=0;
	while(cnt!=2){
		if(*tmp=='\r'){
			cnt++;
			*tmp='\0';
		}
		else if(*tmp=='\n')
			*tmp='\0';
		else
			cnt=0
		tmp++;
	}
	*tmp='\0';
	tmp++;
	req_body=tmp;
	return 0;
}
http_req::int html_analyse(){
	if(req==NULL)
		return -1;
	char *tmp=req_line;
	method=tmp;
	while(*tmp!=' '&&*tmp!='\0'){
		tmp++;
	}
	*tmp='\0';
	tmp++;
	url=tmp;
	while(*tmp!=' '&&*tmp!='\0'){
		tmp++;
	}
	*tmp='\0';
	tmp++;
	version=tmp;
	while(*tmp!='\r')
		tmp++;
	for(int i=0;i<2;i++){
		*tmp='\0';
		tmp++;
	}
	return 0;
}
http_req::int deal_get(){
	char *p=strchr(url,'?');
	if(p!=NULL){
		argv=p+1;		//获取get参数
		p='\0';
		file_path=url;
		if(file_path==NULL)
			return NULL;
		char *tmp=strrchr(file_path,'.');
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
	}
	else
		return -1;
}
