#ifndef __ITEMQUEUE_H__
#define __ITEMQUEUE_H__
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

struct Item
{
    int fd;
    int flags;
};

class ItemQueue
{
public:
  ItemQueue() {};
  ~ItemQueue() {};

  std::shared_ptr<Item> pop() {
      std::lock_guard<std::mutex> lk(mutex_);
      //std::unique_lock<std::mutex> lk(mutex_);
      //cv_.wait(lk, [this]() { return !queue_.empty(); });
      std::shared_ptr<Item>
          item = queue_.front();
      queue_.pop();
      return item;
  };

  void push(std::shared_ptr<Item> item)
  {
      std::lock_guard<std::mutex> lk(mutex_);
      queue_.push(item);
      //cv_.notify_one();
  }

private:
  std::queue<std::shared_ptr<Item>> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

#endif // !__ITEMQUEUE_H__
