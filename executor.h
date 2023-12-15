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
  void run_once() {
    for (auto handle : handles_) {
      if (!handle.done()) {
        printf("resume %p\n", handle.address());
        handle.resume();
      }
    }
  }

  void run() {
    while (!handles_.empty()) {
      printf("\nPOLLING %zu TASKS\n", handles_.size());
      run_once();
      handles_.remove_if([](const auto& handle) { return handle.done(); });
    }
  }

  void schedule(const Handle& handle) override { handles_.push_back(handle); }
  void deschedule(const Handle& handle) override { handles_.remove(handle); }

 private:
  std::list<Handle> handles_;
};

}  // namespace coro::impl
