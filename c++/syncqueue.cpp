#include <iostream>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
#include <random> 
#include <ctime>

std::mutex g_lockprint;


template <typename T>
class SyncQueue {
	mutable std::mutex m;
	std::queue<T> q;
	std::condition_variable c;
	bool n = false;

	static int cnt;
public:
	T pop(){
		// try to lock main mutex
		std::unique_lock<std::mutex> locker(m);
		
		// check queue
		if(q.empty()){
			{
				std::lock_guard<std::mutex> lp(g_lockprint);
				std::cout << "\t\tempty  " << ++cnt << " ...waiting [" << std::this_thread::get_id() << "]" << std::endl;
			}
			while(!n) // block spurious wakeup
				c.wait(locker); // wait notification
		}

		auto e = std::move(q.front());
		q.pop();
		n  = false;

		{
			// print event
			std::lock_guard<std::mutex> lp(g_lockprint);
			std::cout << "\tpop  " << e << " [" << std::this_thread::get_id() << "]" << std::endl;
		}

		return e;
	}

	void push(const T& e){
		// try to lock main mutex
		std::unique_lock<std::mutex> locker(m);

		q.push(e);
		n = true;
		c.notify_one();

		{
			// print event
			std::lock_guard<std::mutex> lp(g_lockprint);
			std::cout << "push " << e << " [" << std::this_thread::get_id() << "]" << std::endl;
		}
	}

	int count() const {
		std::lock_guard<std::mutex> locker(m);
		return q.size();
	}
};

template <typename T>
int SyncQueue<T>::cnt = 0;



void testPush(SyncQueue<int>& q, int aCnt){
	{
		std::lock_guard<std::mutex> lp(g_lockprint);
		std::cout << "entered thread " << std::this_thread::get_id() << std::endl;
	}
	std::mt19937 gen(time(0)); 
    std::uniform_int_distribution<> uid(100, 200);
	for (int i = 0; i < aCnt; i++){
		q.push(i);
		std::this_thread::sleep_for(std::chrono::milliseconds(uid(gen)));
	}
	std::cout << "leaving thread " << std::this_thread::get_id() << std::endl;
}

void testPop(SyncQueue<int>& q, int aCnt){
	{
		std::lock_guard<std::mutex> lp(g_lockprint);
		std::cout << "entered thread " << std::this_thread::get_id() << std::endl;
	}
	std::mt19937 gen(time(0)); 
    std::uniform_int_distribution<> uid(100, 400);
	for (int i = 0; i < aCnt; i++){
		q.pop();
		std::this_thread::sleep_for(std::chrono::milliseconds(uid(gen)));
	}
	std::cout << "leaving thread " << std::this_thread::get_id() << std::endl;
}


int main(){
	SyncQueue<int> q;

	std::cout << "start with " << q.count() << std::endl;

	std::thread t1(testPush, std::ref(q), 100);
    std::thread t2(testPop, std::ref(q), 50);
    std::thread t3(testPop, std::ref(q), 50);
    t1.join();
    t2.join();
    t3.join();

    std::cout << "end with " << q.count() << std::endl;

	return 0;
}