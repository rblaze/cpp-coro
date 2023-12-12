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

  // Rust-style API: run the coroutine and return its value once complete
  std::optional<T> poll() {
    handle_.resume();

    // If coroutine is at final suspension point, delete it and return the
    // value. Or throw its exception.
    if (handle_.done()) {
      handle_.destroy();
      return future_.get();
    }

    return std::nullopt;
  }

 private:
  using Handle = std::coroutine_handle<Promise<T>>;

  std::future<T> future_;
  Handle handle_;
};

// Coroutine internals
template <>
struct Promise<void> : public std::promise<void> {
  Task<void> get_return_object() noexcept { return Task<void>(*this); }

  // Lazy coroutine, does nothing unless polled.
  std::suspend_always initial_suspend() const noexcept { return {}; }
  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present.
  std::suspend_always final_suspend() const noexcept { return {}; }

  void return_void() noexcept { this->set_value(); }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }
};

template <typename T>
struct Promise : public std::promise<T> {
  Task<T> get_return_object() noexcept { return Task(*this); }

  // Lazy coroutine, does nothing unless polled.
  std::suspend_always initial_suspend() const noexcept { return {}; }
  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present.
  std::suspend_always final_suspend() const noexcept { return {}; }

  void return_value(const T& value) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    this->set_value(value);
  }

  void return_value(T&& value) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    this->set_value(std::move(value));
  }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }
};
