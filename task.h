#pragma once

#include <cassert>
#include <coroutine>
#include <exception>
#include <future>

#include "executor.h"
#include "promise.h"

namespace coro::impl {

template <typename T>
class ScheduledTask {
 public:
  ScheduledTask(std::coroutine_handle<Promise<T>>&& handle, Executor& executor)
      : handle_(handle), executor_(executor) {
    printf("ScheduledTask %p %p\n", handle_.address(), this);
    handle_.promise().set_executor(&executor_);
  }

  ~ScheduledTask() {
    if (handle_) {
      printf("~ScheduledTask destroying %p %p\n", handle_.address(), this);
      executor_.deschedule(handle_);
      handle_.destroy();
      handle_ = nullptr;
    }
  }

  T get() && {
    printf("get %p %s %p\n", handle_.address(),
           handle_.done() ? "done" : "NOT_DONE", this);
    return handle_.promise().get_future().get();
  }

  // awaiter implementation
  bool await_ready() const {
    printf("await_ready %p %p\n", handle_.address(), this);
    assert(handle_);
    return handle_.done();
  }

  void await_suspend(std::coroutine_handle<> parent) {
    printf("await_suspend %p %p <- %p\n", handle_.address(), this,
           parent.address());
    executor_.deschedule(parent);
    parent_ = parent;
  }

  T await_resume() {
    printf("await_resume %p %s %p\n", handle_.address(),
           handle_.done() ? "done" : "NOT_DONE", this);
    executor_.schedule(parent_);
    parent_ = nullptr;
    return handle_.promise().get_future().get();
  }

 private:
  std::coroutine_handle<Promise<T>> handle_;
  std::coroutine_handle<> parent_ = nullptr;
  Executor& executor_;
};

// Coroutine Task class.
template <typename T>
class Task {
 public:
  using promise_type = Promise<T>;

  explicit Task(promise_type& promise)
      : handle_(std::coroutine_handle<promise_type>::from_promise(promise)) {
    printf("Task %p %p\n", handle_.address(), this);
  }

  ~Task() {
    if (handle_) {
      printf("~Task destroying %p %p\n", handle_.address(), this);
      handle_.destroy();
      handle_ = nullptr;
    }
  }

  ScheduledTask<T> schedule_on(Executor& executor) && {
    executor.schedule(handle_);
    auto handle = handle_;
    handle_ = nullptr;
    return ScheduledTask<T>(std::move(handle), executor);
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

}  // namespace coro::impl
