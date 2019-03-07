#ifndef __WEB_SERVER_H_
#define __WEB_SERVER_H_

#define PORT 8848		//端口号
#define BACKLOG 10		//等待队列长度
#define BUFFSIZE 1024
#define MAXEVENT 8

bool is_get_http(char* request);
char* get_file_type(char* file_path);
char* get_file_name(char* buff);
int get_file_size(char *file_name);
void deal_http(int sock,char* buff);

#endif
