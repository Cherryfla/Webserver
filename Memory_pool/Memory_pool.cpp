#include <memory.h>  
#include "Memory_pool.h"  

size_t check_align_addr(void*& pBuf){
    size_t align=0;
    size_t addr=(int)pBuf;
    align=ADDR_ALIGN-addr%ADDR_ALIGN;
    pBuf=(char*)pBuf+align;
    return align;
}  
size_t check_align_block(size_t size){
    size_t align=size%MINUNITSIZE;
    return size-align;
}  
size_t check_align_size(size_t size){
    size=(size+MINUNITSIZE-1)/MINUNITSIZE*MINUNITSIZE;
    return size;
}  
/************************************************************************/  
/* 以下是链表相关操作 
/************************************************************************/  

/*   <--next--
    pool    head .... 内存增长方向
      --pre-->
*/
memory_chunk* create_list(memory_chunk* pool, size_t count){
    if(pool==nullptr){//未申请内存
        return nullptr;
    }  
    memory_chunk* head=nullptr;
    for(size_t i=0;i<count;i++){
        pool->pre=nullptr;
        pool->next=head;
        if(head!=nullptr){
            head->pre=pool;
        }  
        head=pool;
        pool++;//?
    }  
    return head;
}  
memory_chunk* pop_front(memory_chunk*& pool){
    if(!pool){
        return nullptr;
    }  
    memory_chunk* tmp=pool;
    pool=tmp->next;
    pool->pre=nullptr;
    return  tmp;
}  
void push_back(memory_chunk*& head, memory_chunk* element){
    if(head==nullptr){//链表为空时 
        head=element;
        head->pre=element;
        head->next=element;
        return;
    }  
    head->pre->next=element;
    element->pre=head->pre;
    head->pre=element;
    element->next=head;
}  
void push_front(memory_chunk*& head, memory_chunk* element){ 
    element->pre=nullptr;
    element->next=head;
    if(head!=nullptr){
        head->pre=element;
    }  
    head=element;
}  
void delete_chunk(memory_chunk*& head, memory_chunk* element){
    // 在双循环链表中删除元素  
    if(element==nullptr){
        return;
    }  
    // element为链表头  
    else if(element==head){
        // 链表只有一个元素  
        if(head->pre==head){
            head=nullptr;
        }  
        else{
            head=element->next;
            head->pre=element->pre;
            head->pre->next=head;
        }  
    }  
    // element为链表尾  
    else if(element->next==head){
        head->pre=element->pre;
        element->pre->next=head;
    }  
    else{
        element->pre->next=element->next;
        element->next->pre=element->pre;
    }  
    element->pre=nullptr;
    element->next=nullptr;
}  

void* idx2addr(Memory_pool* mem_pool, size_t idx){
    char* p=(char*)(mem_pool->memory);
    void* ret=(void*)(p+idx*MINUNITSIZE);
      
    return ret;
}  

size_t addr2idx(Memory_pool* mem_pool, void* addr){
    char* start=(char*)(mem_pool->memory);
    char* p=(char*)addr;
    size_t idx=(p-start)/MINUNITSIZE;
    return idx;
}  
/************************************************************************/  
/* 生成内存池 
* pBuf: 给定的内存buffer起始地址 
* sBufSize: 给定的内存buffer大小 
* 返回生成的内存池指针 
/************************************************************************/  
Memory_pool* CreateMemoryPool(void* pBuf, size_t sBufSize){ 
    memset(pBuf,0,sBufSize);
    Memory_pool* mem_pool=(Memory_pool*)pBuf;
    // 计算需要多少memory map单元格

    size_t mempool_size=sizeof(Memory_pool);//管理单元大小
    size_t block_size=sizeof(memory_block);
    size_t chunk_size=sizeof(memory_chunk);

    mem_pool->pool_cnt=(sBufSize-mempool_size+MINUNITSIZE-1)/MINUNITSIZE;//取上整
    mem_pool->pmem_map=(memory_block*)((char*)pBuf+mempool_size);
    //计算chuck_pool首地址
    mem_pool->pchunk_pool=(memory_chunk*)((char*)pBuf+mempool_size+block_size*mem_pool->pool_cnt);
    //计算实际存储单元的首地址
    mem_pool->memory=(char*)pBuf+mempool_size+block_size*mem_pool->pool_cnt+chunk_size*mem_pool->pool_cnt;
    //计算实际用来存储的大小 
    mem_pool->size=sBufSize-mempool_size-block_size*mem_pool->pool_cnt-chunk_size*mem_pool->pool_cnt;
    size_t align=check_align_addr(mem_pool->memory);//实际存储单元向后移动align个字节
    mem_pool->size-=align;//大小减去align（产生了碎片）
    mem_pool->size=check_align_block(mem_pool->size);//按MINUNITSIZE字节对齐，使得有整数个block  
    mem_pool->block_cnt=mem_pool->size/MINUNITSIZE;//计算含有多少个block
    //创建chunk_pool链表
    mem_pool->pchunk_pool=create_list(mem_pool->pchunk_pool, mem_pool->pool_cnt);
    //初始化 pfree_mem_chunk，双向循环链表  
    memory_chunk* tmp=pop_front(mem_pool->pchunk_pool);//拿出链表的第一个元素用来初始化
    tmp->pre=tmp;
    tmp->next=tmp;
    tmp->pfree_mem_addr=nullptr;
    mem_pool->pool_cnt--;
      
    // 初始化 pmem_map  
    mem_pool->pmem_map[0].count=mem_pool->block_cnt;
    mem_pool->pmem_map[0].pmem_chunk=tmp;
    mem_pool->pmem_map[mem_pool->block_cnt-1].start=0;
    
    tmp->pfree_mem_addr=mem_pool->pmem_map;
    push_back(mem_pool->pfree_mem_chunk,tmp);
    mem_pool->free_chunk_cnt=1;
    mem_pool->mem_used_size=0;
    return mem_pool;
}  
void ReleaseMemoryPool(Memory_pool** ppMem){}  
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
* pMem: 内存池 指针 
* sMemorySize: 要分配的内存大小 
* 成功时返回分配的内存起始地址，失败返回nullptr 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, Memory_pool* pMem){
    sMemorySize=check_align_size(sMemorySize);
    size_t idx=0;
    memory_chunk* tmp=pMem->pfree_mem_chunk;
    for(idx=0;idx<pMem->free_chunk_cnt;idx++){//最佳优先分配
        if(tmp->pfree_mem_addr->count*MINUNITSIZE>=sMemorySize){ 
            break;
        }  
        tmp=tmp->next;
    }  
    if(idx==pMem->free_chunk_cnt){//如果到了最后一个，说明没有足够的空间可以分配
        return nullptr;
    }
    pMem->mem_used_size+=sMemorySize;//使用了的内存加sMemorySize
    if(tmp->pfree_mem_addr->count*MINUNITSIZE==sMemorySize){
        // 当要分配的内存大小与当前chunk中的内存大小相同时，从pfree_mem_chunk链表中删除此chunk  
        size_t current_idx=(tmp->pfree_mem_addr-pMem->pmem_map);
        delete_chunk(pMem->pfree_mem_chunk,tmp);
        tmp->pfree_mem_addr->pmem_chunk=nullptr;

        push_front(pMem->pchunk_pool,tmp);//归还memory_chunk
        pMem->free_chunk_cnt--;
        pMem->pool_cnt++;
        
        return idx2addr(pMem,current_idx);//返回地址
    }  
    else{
        // 当要分配的内存小于当前chunk中的内存时，更改pfree_mem_chunk中相应chunk的pfree_mem_addr  
          
        // 复制当前memory_block  
        memory_block copy;
        copy.count=tmp->pfree_mem_addr->count;
        copy.pmem_chunk=tmp;
        // 记录该block的起始和结束索引  
        memory_block* current_block=tmp->pfree_mem_addr;
        size_t current_idx=(current_block-pMem->pmem_map);

        current_block->count=sMemorySize/MINUNITSIZE;
        pMem->pmem_map[current_idx+current_block->count-1].start=current_idx;//改变最后一个单元的start
        current_block->pmem_chunk=nullptr;// ？nullptr表示当前内存块已被分配  
        
        // 当前block被一分为二，更新剩下的block中的内容
        pMem->pmem_map[current_idx+current_block->count].count=copy.count-current_block->count;
        pMem->pmem_map[current_idx+current_block->count].pmem_chunk=copy.pmem_chunk;//保持在链表中的位置
        // 更新原来的pfree_mem_addr
        tmp->pfree_mem_addr=&(pMem->pmem_map[current_idx+current_block->count]);
      
        size_t end_idx=current_idx+copy.count-1;
        pMem->pmem_map[end_idx].start=current_idx+current_block->count;
        return idx2addr(pMem, current_idx);
    }     
}
/************************************************************************/  
/* 从内存池中释放申请到的内存 
* pMem：内存池指针 
* ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void add_new_chunk(Memory_pool *&pMem,memory_block *&current_block){
    memory_chunk* new_chunk=pop_front(pMem->pchunk_pool);
    new_chunk->pfree_mem_addr=current_block;
    current_block->pmem_chunk=new_chunk;
    push_back(pMem->pfree_mem_chunk, new_chunk);
    pMem->pool_cnt--;
    pMem->free_chunk_cnt++;
}
void FreeMemory(void *ptrMemoryBlock,Memory_pool* pMem){
    size_t current_idx=addr2idx(pMem,ptrMemoryBlock);
    size_t size=pMem->pmem_map[current_idx].count*MINUNITSIZE;
    // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并  
    memory_block* pre_block=nullptr;
    memory_block* next_block=nullptr;
    memory_block* current_block=&(pMem->pmem_map[current_idx]);
    // 第一个  
    if(current_idx==0){
        if (current_block->count<pMem->block_cnt&&next_block->pmem_chunk!=nullptr){//?为什么会比block_cnt大
            next_block=&(pMem->pmem_map[current_idx+current_block->count]);
            // 如果后一个内存块是空闲的，合并  
            next_block->pmem_chunk->pfree_mem_addr=current_block;
            pMem->pmem_map[current_idx+current_block->count+next_block->count-1].start=current_idx;
            current_block->count+=next_block->count;
            current_block->pmem_chunk=next_block->pmem_chunk;
            next_block->pmem_chunk=nullptr;
        }  
        else{
            add_new_chunk(pMem,current_block);
        }         
    }  
    // 最后一个  
    else if(current_idx==pMem->block_cnt-1){
        if(current_block->count<pMem->block_cnt&&pre_block->pmem_chunk!=nullptr){
            pre_block=&(pMem->pmem_map[current_idx-1]);
            size_t idx=pre_block->count;
            pre_block=&(pMem->pmem_map[idx]);
              
            // 如果前一个内存块是空闲的，合并  
            pMem->pmem_map[current_idx+current_block->count-1].start=current_idx-pre_block->count;
            pre_block->count+=current_block->count;
            current_block->pmem_chunk=nullptr;
            // 如果前一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
        }  
        else{
            add_new_chunk(pMem,current_block);
        }  
    }  
    else{ 
        next_block=&(pMem->pmem_map[current_idx+current_block->count]);
        pre_block=&(pMem->pmem_map[current_idx-1]);
        size_t idx=pre_block->start;
        pre_block=&(pMem->pmem_map[idx]);
        bool is_back_merge=false;
        if(next_block->pmem_chunk==nullptr && pre_block->pmem_chunk==nullptr){
            add_new_chunk(pMem,current_block);
        }  
        // 后一个内存块  
        if(next_block->pmem_chunk!=nullptr){
            next_block->pmem_chunk->pfree_mem_addr=current_block;
            pMem->pmem_map[current_idx+current_block->count+next_block->count-1].start=current_idx;
            current_block->count+=next_block->count;
            current_block->pmem_chunk=next_block->pmem_chunk;
            next_block->pmem_chunk=nullptr;
            is_back_merge=true;
        }  
        // 前一个内存块  
        if(pre_block->pmem_chunk!=nullptr){
            pMem->pmem_map[current_idx+current_block->count-1].start=current_idx-pre_block->count;
            pre_block->count+=current_block->count;
            if(is_back_merge){
                delete_chunk(pMem->pfree_mem_chunk, current_block->pmem_chunk);
                push_front(pMem->pchunk_pool, current_block->pmem_chunk);
                pMem->free_chunk_cnt--;
                pMem->pool_cnt++;
            }  
            current_block->pmem_chunk=nullptr;
        }         
    }  
    pMem->mem_used_size-=size;
}  