#include <stdio.h>

#include "task.h"
#include "executor.h"

Task<int> coro() {
    co_return 42;
}

int main() {
    printf("hello world\n");
    return 0;
}
