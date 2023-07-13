#pragma once

#include <mutex>

template<typename T>
class LockStack
{
public:
	LockStack() { }
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		//empty -> top -> pop
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value)	//우아한 방법, condition_variable을 통해
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _stack.empty() == false; });

		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value)
		{

		}

		T data;
		Node* next;
	};

public:
	//[][][][][][][]
	//[head]
	void Push(const T& value)
	{
		//1) 새 노드를 만들고
		//2) 새 노드의 next = head
		//3) head = 새 노드

		Node* node = new Node(value);
		node->next = _head;

		/*if (_head == node->next)
		{
			_head = node;
			return true;
		}
		else
		{
			node->next = _head;
			return false;
		}*/

		//CAS
		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			//다시 시도
			// node->next = _head;
		}

		//이 사이에 새치기 당할 수 있음
		//_head = node;
	}

	bool TryPop(T& value)
	{
		//1) head를 읽는다 (첫단계부터 난관)
		//2) head->next 읽는다
		//3) head = head->next
		//4) data 추출해서 반환
		//5) 추출한 노드를 삭제

		Node* oldHead = _head;
		
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			//다시 시도
			//oldHead = _head;
		}

		if (oldHead == nullptr)
			return false;

		//Exception 고려 X
		value = oldHead->data;
		
		//잠시 삭제 보류
		//delete oldHead;
		//C#, JAVA 같이 GC가 있으면 사실 여기서 끝

		return true;
	}

private:
	atomic<Node*> _head;
};