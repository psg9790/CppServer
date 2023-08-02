#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <thread>
#include <vector>

// 소수 구하기
bool isPrime(int number)
{
	if (number <= 1)
		return false;
	if (number == 2 || number == 3)
		return true;

	for (int i = 2; i < number; i++)
	{
		if ((number % i) == 0)
			return false;
	}
	return true;
}
int CountPrime(int start, int end)
{
	int count = 0;

	for (int i = start; i <= end; i++)
	{
		if (isPrime(i))
			count++;
	}
	return count;
}

// 1과 자기 자신으로만 나뉘면 소수라고 함



int main()
{
	const int MAX_NUMBER = 1'000'000;
	// 1 ~ MAX_NUMBER까지의 소수 개수

	vector<thread> threads;
	int coreCount = thread::hardware_concurrency();	// 병렬로 실행할 수 있는 코어 개수
	int jobCount = (MAX_NUMBER / coreCount) + 1;

	atomic<int> primeCount = 0;

	for (int i = 0; i < coreCount; i++)
	{
		int start = i * jobCount + 1;
		int end = min(MAX_NUMBER, (i + 1) * jobCount);

		threads.push_back(thread([start, end, &primeCount]() {
			primeCount += CountPrime(start, end);
			}));
	}

	for (thread& t : threads)
		t.join();

	cout << primeCount << '\n';
}