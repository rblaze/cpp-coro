#include <stdio.h>

#include "coro.h"

coro::Task<void> void_coro(int id, bool suspend) {
  if (suspend) {
    printf("void_coro %d: suspend\n", id);
    co_await coro::suspend();

    printf("void_coro %d: suspend 2\n", id);
    co_await coro::suspend();
  }

  printf("void_coro %d: return\n", id);
  co_return;
}

coro::Task<int> coro_add(int a, int b) {
  printf("coro_add: returning %d + %d\n", a, b);
  co_return a + b;
}

coro::Task<int> coro_test() {
  // Cocoutines can voluntary yield control
  printf("coro: suspend\n");
  co_await coro::suspend();

  printf("coro: suspend 2\n");
  co_await coro::suspend();

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
    coro::LocalExecutor executor;

    auto task = coro_test().schedule_on(executor);
    auto void_task = void_coro(0, false).schedule_on(executor);

    executor.run();

    printf("\nLOOP DONE\n");
    printf("v = %d\n", std::move(task).get());
  } catch (std::exception& e) {
    printf("exc: %s\n", e.what());
  }

  return 0;
}
