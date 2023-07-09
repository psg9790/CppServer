#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock {
public:
	void lock() {
		//한번에 동작하게끔 유도해주어야 함
		//CAS (Compare-And-Swap)

		bool expected = false;
		bool desired = true;

		//CAS 의사 코드
		/*if (_locked == expected) {
			expected = _locked;	//expected가 _locked로 변경되는 것에 주의
			_locked = desired;
			return true;
		}
		else 
		{
			return flase;
		}*/

		while (_locked.compare_exchange_strong(expected, desired) == false) {
			expected = false;
		}

		/*while (_locked) {
		}
		_locked = true;*/
	}
	void unlock() {
		//_locked = false;
		_locked.store(false);
	}
private:
	atomic<bool> _locked = false;
};

int32 sum = 0;
mutex m;
SpinLock spinLock;

void Add() {
	for (int32 i = 0; i < 100'000;i++) {
		lock_guard<SpinLock> guard(spinLock);
		sum++;
	}
}void Sub() {
	for (int32 i = 0; i < 100'000;i++) {
		lock_guard<SpinLock> guard(spinLock);
		sum--;
	}
}

int main()
{
	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();

	cout << sum << '\n';
}