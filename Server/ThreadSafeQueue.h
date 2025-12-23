#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace msgnet
{

template <typename T>
class ThreadSafeQueue
{
public:
    // Move 버전 - 불필요한 복사 제거
    void push(T &&value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_)
            return;
        queue_.push(std::move(value));
        cv_.notify_one();
    }

    // Emplace - 제자리 생성으로 추가 성능 향상
    template <typename... Args>
    void emplace(Args &&...args)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_)
            return;
        queue_.emplace(std::forward<Args>(args)...);
        cv_.notify_one();
    }

    // 블로킹 pop - move를 사용하여 불필요한 복사 제거
    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]
                 { return !queue_.empty() || shutdown_; });
        if (shutdown_ && queue_.empty())
            return std::nullopt;
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    // 비블로킹 try_pop - 대기하지 않고 즉시 반환
    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
            return std::nullopt;
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    // 큐가 비어있는지 확인 (참고: 멀티스레드에서는 race condition 가능)
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // 큐의 크기 반환 (참고: 멀티스레드에서는 race condition 가능)
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;
};

} // namespace msgnet