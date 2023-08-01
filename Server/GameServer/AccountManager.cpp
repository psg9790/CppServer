#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"
#include "PlayerManager.h"
/*
void AccountManager::ProcessLogin() {
	//AccountLock
	lock_guard<mutex> guard(_mutex);

	//UserLock
	User* user = UserManager::Instance()->GetUser(100);
}
*/

AccountManager GAccountManager;

void AccountManager::AccountThenManager()
{
	WRITE_LOCK;

	this_thread::sleep_for(1s);

	GPlayerManager.Lock();
}

void AccountManager::Lock()
{
	WRITE_LOCK;
}
