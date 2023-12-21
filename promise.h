#pragma once

#include <cassert>
#include <coroutine>
#include <exception>
#include <future>

namespace coro::impl {
// Forward declarations
template <typename T>
class Task;

template <typename T>
class ScheduledTask;

// Coroutine promise.
template <typename T>
class PromiseBase : public std::promise<T> {
 public:
  using ReturnType = T;

  // Coroutines are lazy: they do nothing until scheduled on an executor.
  std::suspend_always initial_suspend() const noexcept { return {}; }

  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present, so we rely on handle.done() instead.
  std::suspend_always final_suspend() const noexcept {
    if (parent_) {
      assert(executor_);
      executor_->schedule(parent_);
    }

    return {};
  }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }

  template <typename U>
  ScheduledTask<U> await_transform(Task<U>&& task) {
    // Schedule child task on the same executor as ourself.
    assert(executor_);
    return std::move(task).schedule_on(*executor_);
  }

  void set_executor(Executor* executor) { executor_ = executor; }
  void set_parent(std::coroutine_handle<> parent) { parent_ = parent; }

 private:
  Executor* executor_ = nullptr;
  std::coroutine_handle<> parent_;
};

// Returning any type except void.
template <typename T>
struct Promise : public PromiseBase<T> {
  Task<T> get_return_object() noexcept { return Task<T>(*this); }

  void return_value(const T& value) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    this->set_value(value);
  }

  void return_value(T&& value) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    this->set_value(std::move(value));
  }
};

// Coroutines returning void are special.
template <>
struct Promise<void> : public PromiseBase<void> {
  Task<void> get_return_object() noexcept;  // { return Task<void>(*this); }

  void return_void() noexcept { this->set_value(); }
};

}  // namespace coro::impl
