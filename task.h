#pragma once

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
      : future_(promise.get_future()), handle_(Handle::from_promise(promise)) {}

  // Run coroutine until it returns control
  bool poll() {
    if (handle_) {
      handle_.resume();

      if (handle_.done()) {
        destroy_handle();
      }
    }

    return !handle_;
  }

  // Get return value once
  T get() && { return future_.get(); }

  bool await_ready() const { return !handle_; }
  
  std::coroutine_handle<> await_suspend(std::coroutine_handle<>) {
    return handle_;
  }

  T await_resume() {
    destroy_handle();
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
  // Lazy coroutine, does nothing unless polled.
  std::suspend_always initial_suspend() const noexcept { return {}; }
  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present.
  std::suspend_always final_suspend() const noexcept { return {}; }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }
};

template <>
struct Promise<void> : public PromiseBase<void> {
  Task<void> get_return_object() noexcept { return Task<void>(*this); }

  void return_void() noexcept { this->set_value(); }
};

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
