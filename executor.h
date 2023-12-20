#pragma once

#include <algorithm>
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
  void run_once();
  void run();

  void schedule(const Handle& handle) override;
  void deschedule(const Handle& handle) override;

 private:
  std::list<Handle> handles_;
};

}  // namespace coro::impl
