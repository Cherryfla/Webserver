#ifndef __TIMER_MNG_
#define __TIMER_MNG_

#include<vector>
#define ull unsigned long long 
#define __INF__ 1e18

using namespace std;

enum t_type {ONCE,CIRCLE};

class Timer_mng;
class Timer;

struct Heap_entry{		//堆条目
	unsigned long long time;
	Timer *timer;
};

class My_vector{	//自己实现的vector
	private:
		static const int SIZE=1024;
		Heap_entry **array;
		int tail;
	public:
		My_vector();
		void push_back(Heap_entry *val);
		~My_vector();
		void pop_back();
		int size();
		int empty();
		Heap_entry*& operator[](int idx);
};
struct Timer{
	t_type ttype;		//任务类型，一次还是循环
	void *(*run)(void *args);	//回调函数
	void *arg;					//会调函数参数
	ull itvl;					//时间片长度(ms)
	ull expires;				//到期时间
	int heapIndex;				//在堆中的下标
	Timer();					//初始化
	void Start(void *(*run)(void *args),void *arg,ull itvl, t_type ttype);
	void OnTimer();				//无剩余时间		
};
struct Timer_mng{
	vector<Heap_entry*> *heap;		//堆
	~Timer_mng();					//析构函数
	void DetectTimers();			//检测是否超时
	void AddTimer(Timer* timer);	//向堆中添加一个定时器
	void RemoveTimer(Timer* timer);	//移除定时器
	void UpHeap(int idx);			//堆操作，向上更新
	void DownHeap(int idx);			//堆操作，向下更新
	void SwapHeap(int idx1, int idx2);	//更换堆中的两个元素
};

ull Get_now();			//得到当前时间(ms)
void *Timers_det(void *arg);
#endif