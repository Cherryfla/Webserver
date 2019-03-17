#include<cstdio>
#include<cstring>
#include<boost/function.hpp>
#include<sys/time.h>


template<typename Fun>
inline void Timer::Start(Fun fun, unsigned interval, TimerType timeType){
    Stop();
    interval=interval;
    timerFun=fun;
    timerType=timeType;
    timer->expires=timer->interval+TimerManager::Get_now();
    manager.AddTimer(this);
}
Timer::Timer(TimerManager& manager):manager(manager),heapIndex(-1){}
Timer::~Timer(){
    Stop();
}
void Timer::Stop(){
    if(heapIndex!=-1){
        manager.RemoveTimer(this);
        heapIndex=-1;
    }
}
 
void Timer::OnTimer(ull Now){
    if(timerType==CIRCLE){
        expires=interval+Now;
        manager.AddTimer(this);
    }
    else{
        heapIndex=-1;
    }
    timerFun();
}
 
//////////////////////////////////////////////////////////////////////////
// TimerManager
 
void TimerManager::AddTimer(Timer* timer){
    timer->heapIndex_ = heap.size();
    HeapEntry entry = { timer->expires,timer };
    heap.push_back(entry);
    UpHeap(heap.size() - 1);
}
 
void TimerManager::RemoveTimer(Timer* timer){
    size_t index = timer->heapIndex_;
    if (!heap.empty() && index < heap.size()){
        if(index == heap.size() - 1){
            heap.pop_back();
        }
        else{
            SwapHeap(index, heap.size() - 1);
            heap.pop_back();
            size_t parent = (index - 1) / 2;
            if(index>0 && heap[index].time < heap[parent].time)
                UpHeap(index);
            else
                DownHeap(index);
        }
    }
}
 
void TimerManager::DetectTimers(){
    ull now = GetCurrentMillisecs();
 
    while (!heap.empty()&&heap[0].time<=now){
        Timer* timer = heap[0].timer;
        RemoveTimer(timer);
        timer->OnTimer(now);
    }
}
 
void TimerManager::UpHeap(size_t idx){
    size_t parent=(idx-1) / 2;
    while (index>0&& heap[idx].time<heap[parent].time){
        SwapHeap(idx, parent);
        index=parent;
        parent=(idx - 1) / 2;
    }
}
 
void TimerManager::DownHeap(size_t index)
{
    size_t child=index* 2 + 1;
    while (child < heap.size()){
        size_t minChild = (child + 1 == heap.size() || heap[child].time < heap[child + 1].time)
            ? child : child + 1;
        if (heap[index].time < heap[minChild].time)
            break;
        SwapHeap(index, minChild);
        index = minChild;
        child = index * 2 + 1;
    }
}
 
void TimerManager::SwapHeap(size_t index1, size_t index2){
    HeapEntry tmp=heap[index1];
    heap[index1]=heap[index2];
    heap[index2]=tmp;
    heap[index1].timer->heapIndex=index1;
    heap[index2].timer->heapIndex=index2;
}
 
 
ull TimerManager::Get_now()
{
#ifdef _MSC_VER
    _timeb timebuffer;
    _ftime(&timebuffer);
    ull ret = timebuffer.time;
    ret = ret * 1000 + timebuffer.millitm;
    return ret;
#else
    timeval tv;         
    ::gettimeofday(&tv, 0);
    ull ret = tv.tv_sec;
    return ret * 1000 + tv.tv_usec / 1000;
#endif
}