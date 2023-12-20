#include "executor.h"

#include <cstdio>

namespace coro::impl {

void LocalExecutor::run_once() {
  for (auto handle : handles_) {
    if (!handle.done()) {
      printf("resume %p\n", handle.address());
      handle.resume();
    }
  }
}

void LocalExecutor::run() {
  while (!handles_.empty()) {
    printf("\nPOLLING %zu TASKS\n", handles_.size());
    run_once();
    handles_.remove_if([](const auto& handle) { return handle.done(); });
  }
}

void LocalExecutor::schedule(const Handle& handle) {
  handles_.push_back(handle);
}
void LocalExecutor::deschedule(const Handle& handle) {
  handles_.remove(handle);
}

}  // namespace coro::impl
