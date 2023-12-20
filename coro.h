#pragma once

#include <cassert>
#include <coroutine>
#include <future>

#include "executor.h"
#include "task.h"
#include "promise_impl.h"

namespace coro {

using impl::Task;
using impl::LocalExecutor;

// Utility types and functions

Task<void> suspend() {
  printf("awaaaaake\n");
  co_return;
}

}  // namespace coro
