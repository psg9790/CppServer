#pragma once

#include <mutex>

template<typename T>
class LockQueue
{
public:
	LockQueue(){ }
	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(value);

		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);

		if (_queue.empty())
			return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _queue.empty() == false; });

		value = std::move(_queue.front());
		_queue.pop();
	}

private:
	mutex _mutex;
	queue<T> _queue;
	condition_variable _condVar;
};


//template<typename T>
//class LockFreeQueue
//{
//	struct Node
//	{
//		shared_ptr<T> data;
//		Node* next = nullptr;
//	};
//
//public:
//	LockFreeQueue() : _head(new Node), _tail(_head)
//	{
//
//	}
//
//	LockFreeQueue(const LockFreeQueue&) = delete;
//	LockFreeQueue& operator=(const LockFreeQueue&) = delete;
//
//	void Push(const T& value)
//	{
//		//stack은 새로운 데이터를 생성하는 과정에서는 경합이 없기 때문에 간단했음
//		//근데 queue는 push를 할 때도 공용의 dummy노드를 접근함
//		shared_ptr<T> newData = make_shared<T>(value);
//
//		Node* dummy = new Node();
//		Node* oldTail = _tail;
//
//		oldTail->data.swap(newData);
//		oldTail->next = dummy;
//
//		_tail = dummy;
//	}
//
//	shared_ptr<T> TryPop()
//	{
//		Node* oldHead = PopHead();
//		if (oldHead == nullptr)
//			return shared_ptr<T>();
//
//		shared_ptr<T> res(oldHead);
//		delete oldHead;
//		return res;
//	}
//
//private:
//	Node* PopHead()
//	{
//		Node* oldHead = _head;
//		if (oldHead == _tail)
//			return nullptr;
//
//		_head = oldHead->next;
//	}
//
//private:
//	//데이터를 넣을 때는 tail, 뽑을 때는 head
//	//더미 노드가 필요
//	Node* _head = nullptr;
//	Node* _tail = nullptr;
//};

template<typename T>
class LockFreeQueue
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount;	//참조권
		Node* ptr = nullptr;

	};
	struct NodeCounter
	{
		uint32 internalCount : 30;	//30bit, 참조권
		uint32 externalCountRemaining : 2;	//2bit, Push & Pop 다중 참조권 관련
	};

	struct Node
	{
		Node()
		{
			NodeCounter newCount;
			newCount.internalCount = 0;
			newCount.externalCountRemaining = 2;	// 0이 되면 Push, Pop 어느 곳에서도 사용하고 있지 않음
			count.store(newCount);
			next.ptr = nullptr;
			next.externalCount = 0;
		}

		void ReleaseRef()
		{
			NodeCounter oldCounter = count.load();

			while (true)
			{
				NodeCounter newCounter = oldCounter;
				newCounter.internalCount--;

				// 끼어들 수 있음
				if (count.compare_exchange_strong(oldCounter, newCounter))
				{
					if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0)
						delete this;

					break;
				}
			}
		}

		atomic<T*> data;
		atomic<NodeCounter> count;
		CountedNodePtr next;
	};

public:
	LockFreeQueue()
	{
		CountedNodePtr node;
		node.ptr = new Node;
		node.externalCount = 1;

		_head.store(node);
		_tail.store(node);
	}

	LockFreeQueue(const LockFreeQueue&) = delete;
	LockFreeQueue& operator=(const LockFreeQueue&) = delete;

	void Push(const T& value)
	{
		////stack은 새로운 데이터를 생성하는 과정에서는 경합이 없기 때문에 간단했음
		////근데 queue는 push를 할 때도 공용의 dummy노드를 접근함
		//shared_ptr<T> newData = make_shared<T>(value);
		//Node* dummy = new Node();
		//Node* oldTail = _tail;
		//oldTail->data.swap(newData);
		//oldTail->next = dummy;
		//_tail = dummy;

		unique_ptr<T> newData = make_unique<T>(value);

		CountedNodePtr dummy;
		dummy.ptr = new Node;
		dummy.externalCount = 1;

		CountedNodePtr oldTail = _tail.load();	//ptr = nullptr

		while (true)
		{
			//참조권 획득 (externalCount를 현시점 기준 +1 한애가 이김)
			IncreaseExternalCount(_tail, oldTail);

			// 소유권을 획득 (data를 먼저 교환한 애가 이김)
			T* oldData = nullptr;
			if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get()))	// 포인터가 아닌 value로 변수를 했을 때는 이 값이 진짜 차지된 여부를 판별하기 힘듬
			{
				oldTail.ptr->next = dummy;
				oldTail = _tail.exchange(dummy);
				FreeExternalCount(oldTail);
				newData.release();	// 데이터에 대한 unique_ptr의 소유권 포기
				break;
			}

			// 소유권 경쟁 패배..
			oldTail.ptr->ReleaseRef();
		}
	}

	shared_ptr<T> TryPop()
	{
		/*Node* oldHead = PopHead();
		if (oldHead == nullptr)
			return shared_ptr<T>();

		shared_ptr<T> res(oldHead);
		delete oldHead;
		return res;*/
	
		CountedNodePtr oldHead = _head.load();

		while (true)
		{
			// 참조권 획득 (externalCount+1 한애가 승리)
			IncreaseExternalCount(_head, oldHead);

			Node* ptr = oldHead.ptr;
			if (ptr == _tail.load().ptr)
			{
				ptr->ReleaseRef();
				return shared_ptr<T>();
			}

			// 소유권 획득 (head = ptr->next)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				T* res = ptr->data.exchange(nullptr);
				FreeExternalCount(oldHead);
				return shared_ptr<T>(res);
			}

			ptr->ReleaseRef();
		}
	}

private:
	static void IncreaseExternalCount(atomic<CountedNodePtr>& counter, CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (counter.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

	static void FreeExternalCount(CountedNodePtr& oldNodePtr)
	{
		Node* ptr = oldNodePtr.ptr;
		const int32 countIncrease = oldNodePtr.externalCount - 2;

		NodeCounter oldCounter = ptr->count.load();

		while (true)
		{
			NodeCounter newCounter = oldCounter;
			newCounter.externalCountRemaining--;
			newCounter.internalCount += countIncrease;

			if (ptr->count.compare_exchange_strong(oldCounter, newCounter))
			{
				if (newCounter.internalCount == 0 & newCounter.externalCountRemaining == 0)
					delete ptr;

				break;
			}
		}
	}

private:
	//데이터를 넣을 때는 tail, 뽑을 때는 head
	//더미 노드가 필요
	atomic<CountedNodePtr> _head;
	atomic<CountedNodePtr> _tail;
};