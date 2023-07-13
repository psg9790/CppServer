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

	void WaitPop(T& value)	//����� ���, condition_variable�� ����
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
		//1) �� ��带 �����
		//2) �� ����� next = head
		//3) head = �� ���

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
			//�ٽ� �õ�
			// node->next = _head;
		}

		//�� ���̿� ��ġ�� ���� �� ����
		//_head = node;
	}

	bool TryPop(T& value)
	{
		//1) head�� �д´� (ù�ܰ���� ����)
		//2) head->next �д´�
		//3) head = head->next
		//4) data �����ؼ� ��ȯ
		//5) ������ ��带 ����

		Node* oldHead = _head;
		
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			//�ٽ� �õ�
			//oldHead = _head;
		}

		if (oldHead == nullptr)
			return false;

		//Exception ��� X
		value = oldHead->data;
		
		//��� ���� ����
		//delete oldHead;
		//C#, JAVA ���� GC�� ������ ��� ���⼭ ��

		return true;
	}

private:
	atomic<Node*> _head;
};