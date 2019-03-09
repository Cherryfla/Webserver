#include<cstdio>
#include"File_ope.h"

int get_file_size(char *file_name){	//得到文件大小	
	if(file_name==NULL)
		return -1;
	FILE *fp=fopen(file_name,"r");
	if(!fp) 
		return -1;
	fseek(fp,0,SEEK_END);
	int size=ftell(fp);
	fclose(fp);
	return size;
}