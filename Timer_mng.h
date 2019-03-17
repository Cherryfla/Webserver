#ifndef __TIMER_MNG_
#define __TIMRR_MNG_

#define ull unsigned long long 

enum TimerType {ONCE,CIRCLE};

class Timer_Mng;
class Timer;

class Timer{
	private:
		friend class Time_Mng;
		Time_Mng& manager;
		TimerType timerType;
		boost::function<void(void)>timerFun;	//回调函数
		unsigned interval;
		ull expires;
		size_t heapIndex;
	public:
		Timer(Time_Mng& manager);
		void OnTimer(ull now);
		template<typename Fun>
		void Start(Fun fun, unsigned interval, TimerType timeType=CIRCLE);
		void Stop();
		
		~Timer();
};

struct Heap_entry{
	unsigned long long time;
	Timer *timer;
}

class Timer_Mng{
	private:
		friend class Timer;
		vector<HeapEntry> heap;
	public:
		static ull Get_now();
		void DetectTimers();
		void AddTimer(Timer* timer);
		void RemoveTimer(Timer* timer);
		void UpHeap(size_t index);
		void DownHeap(size_t index);
		void SwapHeap(size_t, size_t index2);
};
#endif