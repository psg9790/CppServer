#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

vector<int32> v;

//lock
//Mutual Exclusive(상호배타적)
mutex m;

//RAII (Resource Acquisition is Initialization)
template<typename T>
class LockGuard {
public:
    LockGuard(T& m) 
    {
        _mutex = &m;
        _mutex->lock();
    }
    ~LockGuard()
    {
        _mutex->unlock();
    }
private:
    T* _mutex;
}; //std::lock_guard

void Push() {
    //std::lock_guard<std::mutex> lockGuard(m); //즉시 lock

    for (int32 i = 0; i < 10'000; i++) {
       // m.lock();

        std::lock_guard<std::mutex> lockGuard(m); //즉시 lock
        //std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock);
        //uniqueLock.lock();  //잠기는 시점을 뒤로 미룰 수 있음

        v.push_back(i);
        if (i == 1'000)
            break;
        
        //m.unlock();
    }
}


int main()
{
    std::thread t1(Push);
    std::thread t2(Push);

    t1.join();
    t2.join();

    cout << v.size() << '\n';
}
