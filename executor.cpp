#include "executor.h"

#include <cassert>

namespace coro::impl {

bool LocalExecutor::run_once() {
  for (auto job : jobs_) {
    if (!job.suspended && !job.handle.done()) {
      job.handle.resume();
    }
  }

  jobs_.remove_if(
      [](const auto& job) { return job.suspended || job.handle.done(); });

  return !jobs_.empty();
}

void LocalExecutor::run() {
  while (run_once()) {
    // Iterate until no jobs are left.
  }
}

void LocalExecutor::schedule(const Handle& handle) {
  for (auto& job : jobs_) {
    if (job.handle == handle) {
      assert(job.suspended);
      job.suspended = false;
      return;
    }
  }

  // Job not found in the list, add it.
  jobs_.push_back(Job{.handle = handle});
}

void LocalExecutor::deschedule(const Handle& handle) {
  // It's unsafe to remove elements from list while iterating over it, so mark
  // job for later removal
  for (auto& job : jobs_) {
    if (job.handle == handle) {
      job.suspended = true;
      break;
    }
  }
}

}  // namespace coro::impl
