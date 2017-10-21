#pragma once

#include "config.h"
#include <notstd/task.h>
#include <notstd/nslist.h>
#include <mutex>
#include <chrono>

#if defined(PLATFORM_WINDOWS)
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace notstd {

	typedef List<Task*> TaskList;

	template class NOTSTD_API List<Task*>;

	// 派生类需定义
	// void OnDo(ListType listType);
	// void OnRelease(ListType listType);
	template <class ListType, class T>
	class DoubleCacheList
	{
	protected:
		List<ListType*> mList1, mList2;
		List<ListType*> *mAddList, *mDoList;

		//CRITICAL_SECTION mCSForAddTask;
		std::mutex mCSForAddTask;

	public:
		DoubleCacheList()
			: mAddList(&mList1)
			, mDoList(&mList2)
		{
			//::InitializeCriticalSection(&mCSForAddTask);
		}

		virtual ~DoubleCacheList()
		{
			Release();
			//::DeleteCriticalSection(&mCSForAddTask);
		}

		void Release(bool doLeft = false)
		{
			//::EnterCriticalSection(&mCSForAddTask);
			std::lock_guard<std::mutex> locker(mCSForAddTask);
			while (!mList2.IsEmpty()) {
				ListType* v = mList2.RemoveHead();
				T *t = static_cast<T*>(this);
				if (doLeft)
					t->OnDo(v);
				t->OnRelease(v);
			}

			while (!mList1.IsEmpty()) {
				ListType* v = mList1.RemoveHead();
				T *t = static_cast<T*>(this);
				if (doLeft)
					t->OnDo(v);
				t->OnRelease(v);
			}
		}

		void AddItem(ListType *v)
		{
			std::lock_guard<std::mutex> locker(mCSForAddTask);
			mAddList->AddTail(v);
		}

		void JustDoIt(long long timeout)
		{
			T *t = static_cast<T*>(this);
			if (mDoList->IsEmpty())
			{
				std::lock_guard<std::mutex> locker(mCSForAddTask);
				std::swap(mDoList, mAddList);
			}
			if (!timeout)
			{
				while (!mDoList->IsEmpty())
				{
					ListType *listType = mDoList->RemoveHead();
					t->OnDo(listType);
					t->OnRelease(listType);
				}
			}
			else
			{
				//DWORD b = ::timeGetTime();
				std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
				while (!mDoList->IsEmpty())
				{
					ListType *listType = mDoList->RemoveHead();
					t->OnDo(listType);
					t->OnRelease(listType);

					//DWORD e = ::timeGetTime();
					std::chrono::system_clock::time_point e = std::chrono::system_clock::now();
					//if (e - b >= timeout)
					if (std::chrono::duration_cast<std::chrono::microseconds>(e - b).count() >= timeout)
						break;
				}
			}
		}
	};

#if defined(PLATFORM_WINDOWS)
	class NOTSTD_API std::mutex;
#endif
	class NOTSTD_API AsyncTaskManager
		: public DoubleCacheList<Task, AsyncTaskManager>
	{
	public:
		virtual ~AsyncTaskManager();

		void OnDo(Task *task);
		void OnRelease(Task *task);

		void PostTask(Task *task);
		void DoTask(long long time = 0);
	};

}
