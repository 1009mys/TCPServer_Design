#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue
{
public:
    void push(const T &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_)
            return;
        queue_.push(value);
        cv_.notify_one();
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]
                 { return !queue_.empty() || shutdown_; });
        if (shutdown_ && queue_.empty())
            return std::nullopt;
        T val = queue_.front();
        queue_.pop();
        return val;
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;
};
