#ifndef __DEAL_REQ_H_
#define __DEAL_REQ_H_
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>

#define BUFFSIZE 1024
#define KEEPALIVE 1
class http_req{
    private:
        static const int BUFSIZE=128;
        char *req_line,*req_head,*req_body;
        char file_path[BUFSIZE],file_type[BUFSIZE];	//文件相关
        char *method,*url,*version,*argv;//请求行
        int alive,content_length;
        char *host,*charset,*content_type;//请求头
        char *req;		//传入的参数，请求报文
        int sock;		//传入的参数，套接字
    public:
        ~http_req();             //析构函数
        http_req(int n_sock,char *n_req);//初始化函数，传入外部参数
        int req_break();		//将请求分解
        int reqline_analyse();	//将报文行分解
        int deal_get();			//处理get请求
        int deal_post();		//处理post请求
        int http_404(struct stat buf);	//处理文件不存在出错
        int http_403(struct stat buf);	//处理文件访问权限出错

        const char *get_method();
        const char *get_file_path();
        const char *get_file_type();
        const int& get_sock();
};

#endif