#include "threadpool.h"


namespace threadpool
{
	void ThreadPool::Create()
	{
		for (int i = 0; i < CountThread; ++i)
		{
			threads[i] = std::thread(&ThreadPool::Start, this);
		}
	}

	void ThreadPool::Start()
	{
		while (running)
		{
			std::function<void()> task;
			if (!pause && PopTast(task))
			{
				task();
				--task_total;
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

	bool ThreadPool::PopTast(std::function<void()>& task)
	{
		const std::scoped_lock(mtx);
		PQUEUE_TASKS task_Q = TAILQ_FIRST(&QueList);
		if (task_Q == NULL)
			return false;
		task = std::move(task_Q->task);
		TAILQ_REMOVE(&QueList, task_Q, Entries);
		return true;
	}

	//void ThreadPool::PutTask(void(&task)(void))
	//{
	//	++task_total;
	//	{
	//		const std::scoped_lock lock(mtx);
	//		PQUEUE_TASKS que_push = new QUEUE_TASKS;
	//		que_push->task = task;
	//		TAILQ_INSERT_TAIL(&QueList, que_push, Entries);
	//	}
	//}

	void ThreadPool::Delete()
	{
		for (int i = 0; i < CountThread; ++i)
		{
			threads[i].join();
		}
	}

}
