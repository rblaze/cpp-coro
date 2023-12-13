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
      if (!handle_.done()) {
        handle_.resume();
      }

      if (handle_.done()) {
        destroy_handle();
      }
    }

    return !handle_;
  }

  // Get return value once
  T get() && { return future_.get(); }

  bool await_ready() const { return !handle_ || handle_.done(); }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<>) {
    printf("returning handle\n");
    return handle_;
  }

  T await_resume() {
    printf("await_resume\n");
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
