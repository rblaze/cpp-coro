#include <stdio.h>

#include "task.h"

Task<int> coro() { 
  printf("in coro\n");
  co_return 42; 
}

Task<void> void_coro() {
  printf("in void coro\n");
  co_return;
}

int main() {
  try {
    auto task = coro();
    auto void_task = void_coro();
    int v;

    for (;;) {
      printf("polling coro\n");
      auto maybe_v = task.poll();

      if (maybe_v) {
        v = *maybe_v;
        break;
      }
    }

    printf("v = %d\n", v);

    // for (;;) {
    //   printf("polling void coro\n");
    //   auto maybe_v = void_task.poll();

    //   if (maybe_v) {
    //     break;
    //   }
    // }

    // printf("got void\n");
  } catch (std::exception& e) {
    printf("exc: %s\n", e.what());
  }

  return 0;
}
