#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

int32 buffer[10'000][10'000];

int main()
{
	memset(buffer, 0, sizeof(buffer));

	//faster (cache hit이 빈번하게 일어남)
	{
		uint64 start = GetTickCount64();
		int64 sum = 0;
		for (int32 i = 0; i < 10'000; i++)
		{
			for (int32 j = 0; j < 10'000; j++)
			{
				sum += buffer[i][j];
			}
		}
		uint64 end = GetTickCount64();

		cout << "Elapsed Tick: " << (end - start) << '\n';
	}
	//slower
	{
		uint64 start = GetTickCount64();
		int64 sum = 0;
		for (int32 i = 0; i < 10'000; i++)
		{
			for (int32 j = 0; j < 10'000; j++)
			{
				sum += buffer[j][i];
			}
		}
		uint64 end = GetTickCount64();

		cout << "Elapsed Tick: " << (end - start) << '\n';
	}
}