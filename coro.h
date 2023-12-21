#pragma once

#include "executor.h"
#include "task.h"
#include "promise_impl.h"

namespace coro {

using impl::Task;
using impl::LocalExecutor;

// Utility types and functions

Task<void> suspend() {
  // No-op, scheduling this task is enough to suspend the parent.
  co_return;
}

}  // namespace coro
