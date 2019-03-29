#ifndef _Memory_pool_H  
#define _Memory_pool_H  
#include <stdlib.h>  
#define MINUNITSIZE 64  
#define ADDR_ALIGN 8    //对齐的字节

struct memory_chunk; 
struct memory_block{  
    size_t count;                  //
    size_t start; 
    memory_chunk* pmem_chunk; 
}; 
// 可用的内存块结构体  
struct memory_chunk{
    memory_block* pfree_mem_addr; 
    memory_chunk* pre; 
    memory_chunk* next; 
}; 
// 内存池结构体  
typedef struct Memory_pool{  
    void *memory;                  //实际存储单元
    size_t size;                   //实际用来存储的大小
    memory_block* pmem_map;        //已分配内存映射
    memory_chunk* pfree_mem_chunk; //指向空闲空间的指针
    memory_chunk* pchunk_pool;     //memory_chunk池
    size_t mem_used_size;          //记录内存池中已经分配给用户的内存的大小  
    size_t pool_cnt;               //记录memory_chuck pool中剩余chunk个数 
    size_t free_chunk_cnt;         //记录空闲的内存块的数目  
    size_t block_cnt;              // 一个 mem_unit 大小为 MINUNITSIZE  
};

size_t check_align_addr(void*& pBuf);   //pBuf大小补齐到ADDR_ALIGN字节
size_t check_align_block(size_t size);  //size大小减到MINUNITSIZE倍数字节
size_t check_align_size(size_t size);   //size大小补齐到SIZE_ALIGN字节
memory_chunk* create_list(memory_chunk* pool, size_t count);   //创建长度为count的双向链表
memory_chunk* pop_front(memory_chunk*& pool);                  //弹出链表头部指针
void push_back(memory_chunk*& head, memory_chunk* element);    //向链表尾部添加一个元素
void push_front(memory_chunk*& head, memory_chunk* element);   //向链表头部添加一个元素
void delete_chunk(memory_chunk*& head, memory_chunk* element); //删除某个元素
void* index2addr(Memory_pool* mem_pool, size_t index);          //内存映射表中的索引转化为内存起始地址
size_t addr2index(Memory_pool* mem_pool, void* addr);           //内存起始地址转化为内存映射表中的索引
/************************************************************************/  
/* 生成内存池 
 * pBuf: 给定的内存buffer起始地址 
 * sBufSize: 给定的内存buffer大小 
 * 返回生成的内存池指针 
/************************************************************************/  
Memory_pool* CreateMemoryPool(void* pBuf, size_t sBufSize); 
/************************************************************************/  
/* 暂时没用 
/************************************************************************/  
void ReleaseMemoryPool(Memory_pool** ppMem);  
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
 * pMem: 内存池 指针 
 * sMemorySize: 要分配的内存大小 
 * 成功时返回分配的内存起始地址，失败返回NULL 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, Memory_pool* pMem); 
  
/************************************************************************/  
/* 从内存池中释放申请到的内存 
 * pMem：内存池指针 
 * ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void FreeMemory(void *ptrMemoryBlock, Memory_pool* pMem); 
  
#endif //_Memory_pool_H 