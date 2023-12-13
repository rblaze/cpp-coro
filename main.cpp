#include <stdio.h>

#include "task.h"

Task<int> coro() {
  // Voluntary yield control
  printf("coro: suspend\n");
  co_await std::suspend_always{};

  printf("coro: return\n");
  co_return 42;
}

Task<void> void_coro() {
  printf("void_coro: return\n");
  co_return;
}

int main() {
  try {
    auto task = coro();
    auto void_task = void_coro();

    bool task_ready;
    bool void_task_ready;

    do {
      printf("polling coroutines\n");
      task_ready = task.poll();
      void_task_ready = void_task.poll();
    } while (! (task_ready && void_task_ready));

    printf("v = %d\n", std::move(task).get());
  } catch (std::exception& e) {
    printf("exc: %s\n", e.what());
  }

  return 0;
}
