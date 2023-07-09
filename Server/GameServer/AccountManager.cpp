#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin() {
	//AccountLock
	lock_guard<mutex> guard(_mutex);

	//UserLock
	User* user = UserManager::Instance()->GetUser(100);
}