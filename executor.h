#pragma once

#include <future>

class Executor {
 public:
  void spawn(std::future<void>);
  void run_until_complete();
};
