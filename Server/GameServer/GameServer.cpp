#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

mutex m;
queue<int32> q;
HANDLE handle;

void Producer() {
	while (true) {
		unique_lock<mutex> lock(m);
		q.push(100);

		::SetEvent(handle); // Signal = true

		this_thread::sleep_for(100ms);
	}
}

void Consumer() {
	while (true) {
		::WaitForSingleObject(handle, INFINITE);	//handle의 Signal 대기
		//Non-Signal일 때 자동으로 Signal 초기화가 안되고 수동으로 해줘야함
		//::ResetEvent(handle);

		unique_lock<mutex> lock(m);
		if (q.empty() == false) {
			int32 data = q.front();
			q.pop();
			cout << data << '\n';
		}
		else {
			cout << "none" << '\n';
		}
		
	}
}

int main()
{
	//커널 오브젝트 - 유저~커널모드의 개입이 있기 때문에 사용성은 좋음, 하지만 추가적인 비용이 필요함
	//Usage Count
	//Signal / Non-Signal <<- bool
	//Auto / Manual <<- bool
	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*ManualReset*/, FALSE/*InitailState*/, NULL);

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}