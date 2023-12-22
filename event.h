#pragma once

#include <coroutine>
#include <list>

#include "executor.h"

namespace coro {

// Forward declaration.
namespace impl {

class BoundAwaiter;

}  // namespace impl

// Event that coroutine can wait for and external code can signal.
class Event {
 public:
  struct Awaiter {
    using BoundType = impl::BoundAwaiter;
    Event& event;

    BoundType schedule_on(impl::Executor& executor) &&;
  };

  bool signaled() const { return signaled_; }
  Awaiter wait() { return Awaiter{.event = *this}; }
  void signal() {
    signaled_ = true;

    for (auto& waiter : waiters_) {
      waiter.executor.schedule(waiter.handle);
    }
    waiters_.clear();
  }

  void add_waiter(std::coroutine_handle<> handle, impl::Executor& executor) {
    waiters_.push_back(Waiter{.handle = handle, .executor = executor});
  }

 private:
  struct Waiter {
    std::coroutine_handle<> handle;
    impl::Executor& executor;
  };

  bool signaled_ = false;
  std::list<Waiter> waiters_;
};

namespace impl {

class BoundAwaiter {
 public:
  BoundAwaiter(Event& event, Executor& executor)
      : event_(event), executor_(executor) {}

  bool await_ready() { return event_.signaled(); }
  void await_suspend(std::coroutine_handle<> parent) {
    executor_.deschedule(parent);
    event_.add_waiter(parent, executor_);
  }
  void await_resume() { assert(event_.signaled()); }

 private:
  Event& event_;
  Executor& executor_;
};

}  // namespace impl

impl::BoundAwaiter Event::Awaiter::schedule_on(impl::Executor& executor) && {
  return impl::BoundAwaiter(event, executor);
}

}  // namespace coro
