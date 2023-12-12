#pragma once

#include "task.h"

class Executor {
 public:
  void spawn(Task<Unit>);
  void run_until_complete();
};
