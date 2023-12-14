#pragma once

#include <cassert>
#include <coroutine>
#include <future>
#include <optional>

using namespace std::chrono_literals;

template <typename T>
struct Promise;

template <typename T>
class Task {
 public:
  using promise_type = Promise<T>;

  explicit Task(Promise<T>& promise)
      : future_(promise.get_future()), handle_(Handle::from_promise(promise)) {
    printf("Task %p %p\n", handle_.address(), this);
  }

  // Run coroutine until it returns control
  bool poll() {
    if (handle_) {
      if (!handle_.done()) {
        printf("resume %p %p\n", handle_.address(), this);
        handle_.resume();
      }

      if (handle_.done()) {
        destroy_handle();
      }
    }

    printf("polled %p %p\n", handle_.address(), this);

    return !handle_;
  }

  // Get return value once
  T get() && { return future_.get(); }

  // awaiter implementation
  bool await_ready() const {
    printf("await_ready %p %p\n", handle_.address(), this);
    return !handle_ || handle_.done();
  }

  void await_suspend(std::coroutine_handle<> parent) {
    printf("await_suspend %p %p <- %p\n", handle_.address(), this,
           parent.address());
    // Need to add handle_ to parent as blocker, so poll will resume it first.
    // How to get parent Task from parent handle?
  }

  T await_resume() {
    printf("await_resume %p %p\n", handle_.address(), this);
    if (handle_) {
      destroy_handle();
    }
    return future_.get();
  }

 private:
  using Handle = std::coroutine_handle<Promise<T>>;

  void destroy_handle() {
    handle_.destroy();
    handle_ = nullptr;
  }

  std::future<T> future_;
  Handle handle_;
};

// Coroutine internals
template <typename T>
struct PromiseBase : public std::promise<T> {
  // Eager coroutine, runs to the first suspend point or completion.
  // Replace with std::suspend_always to make lazy coroutine than does nothing
  // until polled.
  std::suspend_never initial_suspend() const noexcept { return {}; }
  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present, so we rely on handle.done() instead.
  std::suspend_always final_suspend() const noexcept { return {}; }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }
};

// Coroutines returning void are special.
template <>
struct Promise<void> : public PromiseBase<void> {
  Task<void> get_return_object() noexcept { return Task<void>(*this); }

  void return_void() noexcept { this->set_value(); }
};

// Returning any other type.
template <typename T>
struct Promise : public PromiseBase<T> {
  Task<T> get_return_object() noexcept { return Task(*this); }

  void return_value(const T& value) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    this->set_value(value);
  }

  void return_value(T&& value) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    this->set_value(std::move(value));
  }
};
