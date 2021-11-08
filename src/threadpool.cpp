#include "threadpool.h"
#ifdef BUILD_THREADPOOL
_NAP_BEGIN

using std::unique_lock;
using std::mutex;
using std::thread;

ThreadPool::ThreadPool(size_t threads) {
	for (size_t i = 0; i < threads; ++i) {
		auto handler = [this]() {
			while (true) {
				std::function<void()> task;
				{
					unique_lock<std::mutex> lock(this->queue_mutex);

					this->condition.wait(
						lock,
						[this](){
							return this->stop || !this->tasks.empty(); 
						}
					);


					if (this->stop && this->tasks.empty()) {
						return;
					}else {
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}
				}
				task();
			}
		};

		workers.emplace_back(handler);
	}
}


ThreadPool::~ThreadPool(){
	unique_lock<std::mutex> lock(queue_mutex);
	stop = true;
	lock.unlock();

	condition.notify_all();
	for (thread& worker : workers)
		worker.join();
}


_NAP_END
#endif