#include "nap_common.h"

#include <memory>
#include <condition_variable>
#include <future>
#include <stdexcept>
_NAP_BEGIN

class ThreadPool {
public:
    ThreadPool(size_t);
    ~ThreadPool();

    template<class F, class... Args>
    auto push(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;

    
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;

    // the task queue
    std::queue< std::function<void()> > tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;

};

template<class F, class... Args>
auto ThreadPool::push(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;


    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        if (stop)
            throw "enqueue on stopped ThreadPool";
        std::unique_lock<std::mutex> lock(queue_mutex);


        tasks.emplace(
            [task]() {
                (*task)(); 
            }
        );
    }
    condition.notify_one();
    return res;
}


_NAP_END