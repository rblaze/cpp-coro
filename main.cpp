#include <stdio.h>

#include "task.h"

Task<void> void_coro(int id, bool suspend) {
  if (suspend) {
    printf("void_coro %d: suspend\n", id);
    co_await std::suspend_always{};

    printf("void_coro %d: suspend 2\n", id);
    co_await std::suspend_always{};
  }

  printf("void_coro %d: return\n", id);
  co_return;
}

Task<int> coro_add(int a, int b) {
  printf("coro_add: returning %d + %d\n", a, b);
  co_return a + b;
}

Task<int> coro() {
  // Cocoutines can voluntary yield control
  printf("coro: suspend\n");
  co_await std::suspend_always{};

  printf("coro: suspend 2\n");
  co_await std::suspend_always{};

  printf("coro: await void 1\n");
  co_await void_coro(1, false);

  printf("coro: await void 2\n");
  co_await void_coro(2, true);

  printf("coro: await coro_add\n");
  int ret = co_await coro_add(40, 2);

  printf("coro: return\n");
  co_return ret;
}

int main() {
  try {
    auto task = coro();
    auto void_task = void_coro(0, false);

    bool task_ready;
    bool void_task_ready;

    do {
      printf("\nEXECUTOR: polling coroutines\n");
      task_ready = task.poll();
      void_task_ready = void_task.poll();
    } while (!(task_ready && void_task_ready));

    printf("v = %d\n", std::move(task).get());
  } catch (std::exception& e) {
    printf("exc: %s\n", e.what());
  }

  return 0;
}
