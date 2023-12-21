#pragma once

#include <coroutine>
#include <list>

namespace coro::impl {

class Executor {
 public:
  virtual ~Executor() = default;

  using Handle = std::coroutine_handle<>;
  virtual void schedule(const Handle& handle) = 0;
  virtual void deschedule(const Handle& handle) = 0;
};

class LocalExecutor : public Executor {
 public:
  bool run_once();
  void run();

  void schedule(const Handle& handle) override;
  void deschedule(const Handle& handle) override;

 private:
  struct Job {
    Handle handle;
    bool suspended = false;
  };

  std::list<Job> jobs_;
};

}  // namespace coro::impl
