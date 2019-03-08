#ifndef __DEAL_REQ_H_
#define __DEAL_REQ_H_

struct http_req{
	char *req_line,*req_head,*req_body;
	char *file_path,*file_type;	//文件相关
	char *method,*url,*version,*argv;//请求行
	int alive,content_length;
	char *host,*charset,*content_type;//请求头
	int req_break(char *req);
	int http_analyse(char *req);
	int deal_get(char *req);
}

#endif