#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "AccountManager.h"
#include "PlayerManager.h"

int main()
{
	GThreadManager->Launch([=] {
		while (true)
		{
			cout << "PlayerThenAccount" << '\n';
			GPlayerManager.PlayerThenAccount();
			this_thread::sleep_for(100ms);
		}
		});
	GThreadManager->Launch([=] {
		while (true)
		{
			cout << "AccountThenAccount" << '\n';
			GAccountManager.AccountThenManager();
			this_thread::sleep_for(100ms);
		}
		});
}