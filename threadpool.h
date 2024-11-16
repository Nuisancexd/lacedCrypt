#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "queue.h"
#include <atomic>    
#include <chrono>      
#include <functional>  
#include <mutex>       
#include <thread>      


namespace threadpool
{

	struct ThreadPool
	{
	private:
		typedef struct queue_tasks_
		{
			std::function<void()> task;
			TAILQ_ENTRY(queue_tasks_) Entries;
		} QUEUE_TASKS, * PQUEUE_TASKS;

		TAILQ_HEAD(, queue_tasks_) QueList;


		int CountThread;
		mutable std::mutex mtx;
		std::unique_ptr<std::thread[]> threads;
		std::atomic<bool> running = true;
		std::atomic<int> task_total;

	public:
		explicit ThreadPool(int CountThread_) : CountThread(CountThread_), threads(new std::thread[CountThread_])
		{
			TAILQ_INIT(&QueList);
			Create();
		}

		~ThreadPool()
		{
			while (true)
			{
				if (!pause && task_total == 0)
					break;
				std::this_thread::yield();
			}
			running = false;
			while (TAILQ_FIRST(&QueList) != NULL)
				TAILQ_REMOVE(&QueList, TAILQ_FIRST(&QueList), Entries);
			Delete();
		}

		ThreadPool& const operator=(ThreadPool const&) = delete;
		ThreadPool(ThreadPool const&) = delete;

	private:
		void Create();
		void Start();
		bool PopTast(std::function<void()>& task);
		void Delete();

	public:
		template <typename F>
		void PutTask(const F& task)
		{
			++task_total;
			{
				const std::scoped_lock lock(mtx);
				PQUEUE_TASKS que_push = new QUEUE_TASKS;
				que_push->task = task;
				TAILQ_INSERT_TAIL(&QueList, que_push, Entries);
			}
		}

		template <typename F, typename... A>
		void PutTask(const F& task, const A &...args)
		{
			PutTask([task, args...]
				{ task(args...); });
		}
		//void PutTask(void(&task)(void));
		//void PutTask(void(&task)(const void*& arg...), const void*& args, ...);
		std::atomic<bool> pause = false;
	};
}

#endif
