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

//참고) CV는 User-Level Object (커널 오브젝트 X)
condition_variable cv;

void Producer() {
	while (true) {
		//1) Lock을 잡고
		//2) 공유 변수 값을 수정
		//3) Lock을 풀고,
		//4) 조건변수(condition_variable) 통해 다른 쓰레드에게 통지

		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		// 여기 나오면 lock 파괴되는듯?
		
		cv.notify_one(); //wait중인 쓰레드 중에서 딱 1개를 깨운다

		this_thread::sleep_for(100ms);
	}
}

void Consumer() {
	while (true) {
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() {return q.empty() == false;});	//내부적으로 조건을 충족하지 않으면 일시적으로 unlock 한 뒤, notify를 대기함: 
		// notify 수신 시 lock을 다시 걸고 조건을 확인하는 절차를 밟을 것으로 예상

		//1) Lock을 잡고
		//2) 조건 확인
		//-만족0 => 빠져 나와서 이어서 코드를 진행
		//-만족x => Lock을 풀어주고 대기 상태 (풀어줘야 하기 때문에 unique_lock 사용)

		//while (q.empty() == false) 
		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << '\n';
		}
	}
}

int main()
{
	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();
}