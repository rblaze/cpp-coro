#pragma once

#include <cassert>
#include <coroutine>
#include <exception>
#include <future>

namespace coro::impl {
// Coroutines could be eager or lazy.
// Eager coroutines run until first suspend point or completion.
// Lazy coroutines do nothing until polled.
enum class CoroutineEagerness { Eager, Lazy };

// Forward declarations
template <typename T, CoroutineEagerness E>
class Task;

template <typename P>
class ScheduledTask;

// Forward declarations
template <typename T, CoroutineEagerness E>
struct Promise;

template <typename T>
using EagerPromise = Promise<T, CoroutineEagerness::Eager>;

template <typename T>
using LazyPromise = Promise<T, CoroutineEagerness::Lazy>;

// Type traits for eager and lazy coroutines.
template <CoroutineEagerness, typename T>
struct EagernessTraits {};

template <typename T>
struct EagernessTraits<CoroutineEagerness::Eager, T> {
  using TaskType = Task<T, CoroutineEagerness::Eager>;
  using PromiseType = EagerPromise<T>;
  using InitialSuspendType = std::suspend_never;
};

template <typename T>
struct EagernessTraits<CoroutineEagerness::Lazy, T> {
  using TaskType = Task<T, CoroutineEagerness::Lazy>;
  using PromiseType = LazyPromise<T>;
  using InitialSuspendType = std::suspend_always;
};

// Coroutine promise.
template <typename T, CoroutineEagerness E>
class PromiseBase : public std::promise<T> {
 public:
  using ReturnType = T;

  typename EagernessTraits<E, T>::InitialSuspendType initial_suspend()
      const noexcept {
    return {};
  }

  // Delay coroutine destruction until requested: std::future lacks API to check
  // if value is present, so we rely on handle.done() instead.
  std::suspend_always final_suspend() const noexcept { return {}; }

  void unhandled_exception() noexcept {
    this->set_exception(std::current_exception());
  }

  template <typename U>
  ScheduledTask<EagerPromise<U>> await_transform(
      Task<U, CoroutineEagerness::Eager>&& task) {
    return std::move(task).schedule_on(*executor_);
  }

  template <typename U>
  ScheduledTask<LazyPromise<U>> await_transform(
      Task<U, CoroutineEagerness::Lazy>&& task) {
    return std::move(task).schedule_on(*executor_);
  }

  void set_executor(Executor* executor) { executor_ = executor; }

 private:
  Executor* executor_ = nullptr;
};

// Returning any other type.
template <typename T, CoroutineEagerness E>
struct Promise : public PromiseBase<T, E> {
  using TaskType = typename EagernessTraits<E, T>::TaskType;

  TaskType get_return_object() noexcept { return TaskType(*this); }

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
template <CoroutineEagerness E>
struct Promise<void, E> : public PromiseBase<void, E> {
  using TaskType = typename EagernessTraits<E, void>::TaskType;

  TaskType get_return_object() noexcept { return TaskType(*this); }

  void return_void() noexcept { this->set_value(); }
};

}  // namespace coro::impl
