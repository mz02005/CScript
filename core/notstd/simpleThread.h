#pragma once
#include "config.h"
#include "simpleTool.h"
#include "AsynTask.h"

namespace notstd {
#if defined(PLATFORM_WINDOWS)
	class NOTSTD_API SimpleThread
	{
	public:
		typedef void(*OnThreadProc)(void *lParam);

	private:
		void *mUser;
		Handle<NormalHandleType> mThread;
		Handle<NormalHandleType> mStopEvent;
		//Handle<LONG(NULL)> mThread;
		//Handle<LONG(NULL)> mStopEvent;
		OnThreadProc mOnThread;

	private:
		static UINT WINAPI ThreadProcInner(LPVOID lParam);

	public:
		SimpleThread();
		~SimpleThread();

		bool startThread(OnThreadProc onThread, void *lParam);
		void stopThread();
		bool isStopped();
		bool shouldIStop(unsigned long timeout = 0);

		void* GetUserData() { return mUser; }

		operator HANDLE() { return mThread; }
		HANDLE GetStopSignalHandle() { return mStopEvent; }
	};

#define DECLARE_THREAD(theThread) \
	private: \
		SimpleThread m##theThread; \
		static void theThread##Proc(void *lParam); \
		void On##theThread(SimpleThread *thread); \
	public: \
		bool Start##theThread();

#define IMPLEMENT_THREAD(classname,theThread) \
	void classname::theThread##Proc(void *lParam) { \
		SimpleThread *thread = reinterpret_cast<SimpleThread*>(lParam); \
		classname *c = reinterpret_cast<classname*>(thread->GetUserData()); \
		c->On##theThread(thread); \
		} \
	bool classname::Start##theThread() { \
		return m##theThread.startThread(&classname::theThread##Proc, this); \
		}

	// 能够处理Task的线程
	class NOTSTD_API TaskThread
	{
	protected:
		SimpleThread mThread;
		AsyncTaskManager mTaskManager;

		static void OnTaskThread(void *thread);
		void TaskThreadProc();
		virtual void OnIdle(DWORD tick);

	public:
		TaskThread();
		virtual ~TaskThread();

		SimpleThread* GetThread() { return &mThread; }
		bool startTaskThread();
		void stopTaskThread();

		void AddTask(Task *task) { mTaskManager.AddItem(task); }
	};
#endif
}
