#pragma once

#include <cassert>
#include <coroutine>
#include <future>

#include "executor.h"
#include "task.h"

namespace coro {

template <typename T>
using EagerTask = impl::Task<T, impl::CoroutineEagerness::Eager>;

template <typename T>
using LazyTask = impl::Task<T, impl::CoroutineEagerness::Lazy>;

using impl::LocalExecutor;

// Utility types and functions

LazyTask<void> suspend() {
  printf("awaaaaake\n");
  co_return;
}

}  // namespace coro
