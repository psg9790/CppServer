#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

//가시성, 코드 재배치
//가시성: 각 코어마다 다른 캐시를 사용하기 때문에 발생
//코드 재배치: 컴파일러나 CPU의 자체 판단에 따라 코드 순서가 뒤바뀔 수도 있음
int32 x = 0;
int32 y = 0;
int32 r1 = 0;
int32 r2 = 0;

volatile bool ready;

void thread_1()
{
	while (!ready) {

	}
	//r1 = x;
	y = 1;
	r1 = x;
}

void thread_2()
{
	while (!ready) {

	}
	//r2 = y;
	x = 1;
	r2 = y;
}

int main()
{
	int32 count = 0;
	while (true)
	{
		ready = false;
		count++;
		x = y = r1 = r2 = 0;
		thread t1(thread_1);
		thread t2(thread_2);

		ready = true;

		t1.join();
		t2.join();

		if (r1 == 0 && r2 == 0) {
			break;
		}
	}
	cout << count << "번만에 빠져나옴" << '\n';
}