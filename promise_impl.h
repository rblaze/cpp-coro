#pragma once

#include "promise.h"
#include "task.h"

namespace coro::impl {

Task<void> Promise<void>::get_return_object() noexcept {
  return Task<void>(*this);
}

}  // namespace coro::impl
