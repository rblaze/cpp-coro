#include <stdio.h>

#include "executor.h"
#include "task.h"

Task<int> coro() { co_return 42; }

int main() {
  printf("creating coro\n");
  auto task = coro();
  int v;

  for (;;) {
    printf("waiting\n");
    auto maybe_v = task.poll();

    if (maybe_v) {
        v = *maybe_v;
        break;
    }
  }

  printf("v = %d\n", v);

  return 0;
}
