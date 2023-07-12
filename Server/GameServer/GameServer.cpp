#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

atomic<bool> flag;

int main()
{
	flag = false;

	//cout << flag.is_lock_free() << '\n';	//애초에 원자적으로 처리가 된다면 true를 반환함 (Lock이 필요함)

	flag.store(true, memory_order::memory_order_seq_cst);

	bool val = flag.load();

	//이전 flag 값을 prev에 넣고, flag 값을 수정
	{
		bool prev = flag.exchange(true);
		/*bool prev = flag;
		flag = true;*/
	}

	//CAS (Compare-And-Swap)
	{
		bool expected = false;
		bool desired = true;
		flag.compare_exchange_strong(expected, desired);

		/*if (flag == expected)
		{
			//in _weak: 다른 쓰레드의 interruption을 받아서 중간에 실패할 수 있음
			flag = desired;
			return true;
		}
		else
		{
			expected = flag;
			return false;
		}*/

		flag.compare_exchange_weak(expected, desired);
	}
}