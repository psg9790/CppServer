#include "pch.h"
#include "CorePch.h"
#include "ThreadManager.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

CoreGlobal Core;

void ThreadMain()
{
	while (true)
	{
		cout << "Hello I am thread... " << LThreadId << '\n';
		this_thread::sleep_for(1s);
	}
}
int main()
{
	for (int i = 0; i < 5; i++)
	{
		GThreadManager->Launch(ThreadMain);
	}
	GThreadManager->Join();
}