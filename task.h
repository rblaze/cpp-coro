#pragma once

#include <coroutine>
#include <future>
#include <optional>

struct Unit {};

template <typename T>
class Task {
 public:
  explicit Task(std::promise<T>& promise)
      : future_(promise.get_future()), handle_(Handle::from_promise(promise)) {}

  std::optional<T> poll() {
    handle_.resume();

    if (handle_.done()) {
      return future_.get();
    }

    return std::nullopt;
  }

  using Promise = std::promise<T>;

 private:
  using Handle = std::coroutine_handle<Promise>;

  std::future<T> future_;
  std::coroutine_handle<Promise> handle_;
};

template <typename T, typename... Args>
//   requires(!std::is_void_v<T> && !std::is_reference_v<T>)
struct std::coroutine_traits<Task<T>, Args...> {
  struct promise_type : Task<T>::Promise {
    Task<T> get_return_object() noexcept { return Task(*this); }

    // Lazy coroutine, does nothing unless polled.
    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() const noexcept { return {}; }

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
};
