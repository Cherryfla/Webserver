#include<cstdio>
#include<cstring>
#include<sys/time.h>
#include<iostream>
#include"Timer_mng.h"
#include"WebServer.h"
using namespace std;
//My_vector
My_vector::My_vector(){
    tail=0;
    array=new Heap_entry* [SIZE];
}
My_vector::~My_vector(){delete [] array;}
void My_vector::push_back(Heap_entry* val){
    if(tail>=SIZE)
        return ;
    array[tail++]=val;
}
void My_vector::pop_back(){--tail;}
int My_vector::size(){return tail;}
int My_vector::empty(){return tail==0;}
Heap_entry* My_vector::operator[](const int idx)const {return array[idx];}
void My_vector::set_val(int idx,Heap_entry *val){array[idx]=val;}

//Timer
Timer::Timer(){
    heapIndex=-1;
}
void Timer::Start(void *(*run)(void *args),void *arg,ull itvl, t_type ttype){
    itvl=itvl;
    this->run=run,this->arg=arg;
    ttype=ttype;
    printf("Current time is(ms): %llu\n",Get_now());
    expires=itvl+Get_now();
}
 
void Timer::OnTimer(){
    heapIndex=-1;
   // printf("%d\n",*(int*)arg);
    this->run(this->arg);
}
 
//Timer_mng

Timer_mng::~Timer_mng(){}

void Timer_mng::AddTimer(Timer* timer){  
    timer->heapIndex=heap->size();
    Heap_entry *entry=(Heap_entry*)malloc(sizeof(Heap_entry));
    entry->time=timer->expires;
    entry->timer=timer;
    //printf("q q q q\n");
//    timer->run(timer->arg);
    heap->push_back(entry);
    
    UpHeap(heap->size()-1);

 //   free(entry);
}
 
void Timer_mng::RemoveTimer(Timer* timer){
    int idx=timer->heapIndex;
    if (!heap->empty()&&idx<heap->size()){
        if(idx==heap->size()-1){
            heap->pop_back();
        }
        else{
            SwapHeap(idx,heap->size()-1);
            heap->pop_back();
            int parent=(idx-1)/2;  //堆中父节点编号
            if(idx>0&&(*heap)[idx]->time<(*heap)[parent]->time)
                UpHeap(idx);
            else
                DownHeap(idx);
        }
    }
}
 
void Timer_mng::DetectTimers(){
   // printf("----%lu\n",heap->size());
    if(heap->empty())
        return ;
    ull now=Get_now();
    if((*heap)[0]->time<=now){
        Timer *timer=(Timer *)malloc(sizeof(Timer));
        timer=(*heap)[0]->timer;
  //      printf("%d\n",*(int *)(*heap)[0]->timer->arg);
        if((*heap)[0]->timer->run==NULL||(*heap)[0]->timer->arg==NULL){
            perror("Function");
            return;
        }

       // (*heap)[0]->timer->run((*heap)[0]->timer->arg);
        RemoveTimer(timer);
        if(timer->ttype==CIRCLE){
            timer->expires=timer->itvl+Get_now();
            this->AddTimer(timer);
        }
        timer->OnTimer();
       // printf("xxx\n");
    }
}
 
void Timer_mng::UpHeap(int idx){
    int parent=(idx-1)/2;
    while(idx>0&&(*heap)[idx]->time<(*heap)[parent]->time){
        SwapHeap(idx,parent);
        idx=parent;
        parent=(idx-1)/2;
    }
}
 
void Timer_mng::DownHeap(int idx){
    int child=idx*2+1;
    while(child<heap->size()){
        int minChild=(child+1==heap->size()||(*heap)[child]->time<(*heap)[child+1]->time)
            ?child:child+1;
        if((*heap)[idx]->time<(*heap)[minChild]->time)
            break;
        SwapHeap(idx,minChild);
        idx=minChild;
        child=idx*2+1;
    }
}
 
void Timer_mng::SwapHeap(int idx1, int idx2){
    Heap_entry *tmp=(*heap)[idx1];
    (*heap)[idx1]=(*heap)[idx2];//->set_val(idx1,(*heap)[idx2]);
    (*heap)[idx2]=tmp;//set_val(idx2,tmp);
    (*heap)[idx1]->timer->heapIndex=idx1;
    (*heap)[idx2]->timer->heapIndex=idx2;
}

ull Get_now(){
    #ifdef _MSC_VER
        _timeb timebuffer;
        _ftime(&timebuffer);
        ull ret=timebuffer.time;
        ret=ret*1000+timebuffer.millitm;
        return ret;
    #else
        timeval tv;         
        ::gettimeofday(&tv,0);
        ull ret=tv.tv_sec;
        return ret*1000+tv.tv_usec/1000;
    #endif
}

void *Timers_det(void *arg){
    if(arg==nullptr)
        return nullptr;
    Mng_union *mng_union=(Mng_union *)arg;
    if(mng_union->mutex==nullptr||mng_union->manager==nullptr)
        return nullptr;
    while(true){
        mng_union->mutex->lock();
        if(mng_union->manager->heap->size()==0)
            mng_union->mutex->wait();
        mng_union->manager->DetectTimers();
        mng_union->mutex->unlock();
    }
    return nullptr;
}
// init()

// AddTimer()---->UpHeap()
      
// DetectTimers()----->RemoveTimer()--->UpHeap()------|
//              |             |    |                  |
//              |             |    --->DownHeap()-----|
//              |             |                       |
//              |             ---------------------->SwapHeap()
//              |
//              --->OnTimer()(AddTimer())