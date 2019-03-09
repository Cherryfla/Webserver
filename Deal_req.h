#ifndef __DEAL_REQ_H_
#define __DEAL_REQ_H_
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>

struct http_req{
	static const int BUFSIZE=128;
	char *req_line,*req_head,*req_body;
	char file_path[BUFSIZE],file_type[BUFSIZE];	//文件相关
	char *method,*url,*version,*argv;//请求行
	int alive,content_length;
	char *host,*charset,*content_type;//请求头
	int req_break(char *req);	//将请求分解
	int reqline_analyse();	//将报文行分解
	int deal_get();	//处理get请求
	int deal_post();//处理post请求
	int deal_req(int sock,char* req);
};
#endif