#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

int64 Calculate()
{
	int64 sum = 0;
	for (int32 i = 0; i < 100'000; i++)
	{
		sum += i;
	}
	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("secret message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task) 
{
	task();
}

int main()
{
	//동기(synchronous) 실행
	int64 sum = Calculate();
	cout << sum << '\n';

	
	//std::future
	{
		//1) deferred -> lazy evaluation 지연해서 실행하세요
		//2) async -> 별도의 쓰레드를 만들어서 실행하세요
		//3) deferred | async -> 둘 중 알아서 골라주세요

		//언젠가 미래에 결과물을 뱉어줄거야!
		std::future<int64> future = std::async(std::launch::async, Calculate);	//코루틴이랑 비슷?

		//TODO
		/*future_status status = future.wait_for(1ms);
		if (status == future_status::ready)
		{

		}*/

		int64 sum = future.get();	//결과물이 이제서야 필요하다!

		/*class Knight {
		public:
			int64 getHp() { return 100; }
		};
		Knight knight;
		std::future<int64> future2 = std::async(std::launch::async, Knight::getHp, knight);*/
	}

	//std::promise
	{
		//미래(future)에 결과물을 반환해줄거라 약속(std::promise) 해줘~ (계약서?)
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string str = future.get();
		cout << str << '\n';

		t.join();
	}

	//std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << '\n';

		t.join();
	}

	//결론)
	//mutex, condition_variable까지 가지 않고 단순한 애들을 처리할 수 있는
	//특히나, 한번 발생하는 단발성 이벤트에 유용하다.
	//닭 잡는데 소 잡는 칼을 쓸 필요 없다.
	//1) async
	//원하는 함수를 비동기적으로 실행
	//2) promise
	//결과물을 promise를 통해 future로 받아줌
	//3) packaged_task
	//원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌
}