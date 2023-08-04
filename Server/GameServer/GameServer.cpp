﻿#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include "RefCounting.h"


class Wraith : public RefCountable
{
public:
	int _hp = 150;
	int _posX = 0;
	int _posY = 0;
};

using WraithRef = TSharedPtr<Wraith>;
class Missile : public RefCountable
{
public:
	void SetTarget(WraithRef target)
	{
		_target = target;
		// 중간에 개입 가능 : 레퍼 추가하기 전에 레이스가 죽어버리면?
		//target->AddRef(); // 참조 증가
	}
	bool Update()
	{
		if (_target == nullptr)
			return false;

		int posX = _target->_posX;
		int posY = _target->_posY;

		// TODO: 쫓아간다

		if (_target->_hp == 0)
		{
			_target->ReleaseRef();
			_target = nullptr;
			return false;
		}

		return true;
	}

	WraithRef _target = nullptr;
};
using MissileRef = TSharedPtr<Missile>;



int main()
{
	WraithRef wraith(new Wraith());
	wraith->ReleaseRef();
	MissileRef missile(new Missile());
	missile->ReleaseRef();

	missile->SetTarget(wraith);

	// 레이스가 피격당함
	wraith->_hp = 0;
	//delete wraith;
	//wraith->ReleaseRef();
	wraith = nullptr;

	while (true)
	{
		if (missile)
		{
			if (!missile->Update())
			{
				//missile->ReleaseRef();
				missile = nullptr;
			}
		}
	}
	//delete missile;
	//missile->ReleaseRef();
	missile = nullptr;
}