#include <stdio.h>

#include "task.h"

Task<void> void_coro(int);

Task<int> coro_add(int a, int b) {
  printf("coro_add: returning %d + %d\n", a, b);
  co_return a + b;
}

Task<int> coro() {
  // Voluntary yield control
  printf("coro: suspend\n");
  co_await std::suspend_always{};

  printf("coro: await void\n");
  co_await void_coro(1);

  printf("coro: await coro_add\n");
  int ret = co_await coro_add(40, 2);

  printf("coro: return\n");
  co_return ret;
}

Task<void> void_coro(int id) {
  printf("void_coro %d: return\n", id);
  co_return;
}

int main() {
  try {
    auto task = coro();
    auto void_task = void_coro(0);

    bool task_ready;
    bool void_task_ready;

    do {
      printf("polling coroutines\n");
      task_ready = task.poll();
      void_task_ready = void_task.poll();
    } while (!(task_ready && void_task_ready));

    printf("v = %d\n", std::move(task).get());
  } catch (std::exception& e) {
    printf("exc: %s\n", e.what());
  }

  return 0;
}
