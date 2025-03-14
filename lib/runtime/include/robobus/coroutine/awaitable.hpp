#pragma once

#include "robobus/coroutine/coroutine.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"
namespace robobus::coroutine {

template <typename T, typename RetT>
concept Awaitable = CoroutineLike<T, RetT> or AwaiterLike<T, RetT>;

}