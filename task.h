#pragma once

#include <cassert>
#include <coroutine>

#include "executor.h"
#include "promise.h"

namespace coro::impl {

template <typename T>
class ScheduledTask {
 public:
  ScheduledTask(std::coroutine_handle<Promise<T>>&& handle, Executor& executor)
      : handle_(handle), executor_(executor) {
    handle_.promise().set_executor(&executor_);
  }

  ~ScheduledTask() {
    if (handle_) {
      executor_.deschedule(handle_);
      handle_.destroy();
      handle_ = nullptr;
    }
  }

  T get() && { return handle_.promise().get_future().get(); }

  // awaiter implementation
  bool await_ready() const {
    assert(handle_);
    return handle_.done();
  }

  void await_suspend(std::coroutine_handle<> parent) {
    executor_.deschedule(parent);
    handle_.promise().set_parent(parent);
  }

  T await_resume() { return handle_.promise().get_future().get(); }

 private:
  std::coroutine_handle<Promise<T>> handle_;
  Executor& executor_;
};

// Coroutine Task class.
template <typename T>
class Task {
 public:
  using promise_type = Promise<T>;
  using BoundType = ScheduledTask<T>;

  explicit Task(promise_type& promise)
      : handle_(std::coroutine_handle<promise_type>::from_promise(promise)) {}

  ~Task() {
    if (handle_) {
      handle_.destroy();
      handle_ = nullptr;
    }
  }

  BoundType schedule_on(Executor& executor) && {
    executor.schedule(handle_);
    auto handle = handle_;
    handle_ = nullptr;
    return ScheduledTask<T>(std::move(handle), executor);
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

}  // namespace coro::impl
