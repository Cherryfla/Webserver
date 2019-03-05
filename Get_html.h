#ifndef __GET_HTML_H_
#define __GET_HTML_H_

#define PORT 8848
#define BACKLOG 10
#define BUFFSIZE 1024

bool Get_http(char* buff);
char* get_file_name(char* buff);
int get_file_size(char *file_name);
void deal_http(int sock,char* buff);

#endif
