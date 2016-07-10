#pragma once

#include "config.h"
#include <notstd/task.h>
#include <notstd/nslist.h>
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

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

		CRITICAL_SECTION mCSForAddTask;

	public:
		DoubleCacheList()
			: mAddList(&mList1)
			, mDoList(&mList2)
		{
			::InitializeCriticalSection(&mCSForAddTask);
		}

		virtual ~DoubleCacheList()
		{
			Release();
			::DeleteCriticalSection(&mCSForAddTask);
		}

		void Release(bool doLeft = false)
		{
			::EnterCriticalSection(&mCSForAddTask);
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
			::LeaveCriticalSection(&mCSForAddTask);
		}

		void AddItem(ListType *v)
		{
			::EnterCriticalSection(&mCSForAddTask);
			mAddList->AddTail(v);
			::LeaveCriticalSection(&mCSForAddTask);
		}

		void JustDoIt(DWORD timeout)
		{
			T *t = static_cast<T*>(this);
			if (mDoList->IsEmpty())
			{
				::EnterCriticalSection(&mCSForAddTask);
				std::swap(mDoList, mAddList);
				::LeaveCriticalSection(&mCSForAddTask);
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
				DWORD b = ::timeGetTime();
				while (!mDoList->IsEmpty())
				{
					ListType *listType = mDoList->RemoveHead();
					t->OnDo(listType);
					t->OnRelease(listType);

					DWORD e = ::timeGetTime();
					if (e - b >= timeout)
						break;
				}
			}
		}
	};

	class NOTSTD_API AsyncTaskManager
		: public DoubleCacheList<Task, AsyncTaskManager>
	{
	public:
		virtual ~AsyncTaskManager();

		void OnDo(Task *task);
		void OnRelease(Task *task);

		void PostTask(Task *task);
		void DoTask(DWORD timeout = 0);
	};

}
