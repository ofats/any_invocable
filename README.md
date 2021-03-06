# any_invocable
One of possible implementations of std::any_invocable proposed in [P0288R5](https://wg21.link/P0288R5).

This paper proposes a conservative, move-only equivalent of std::function.

### Current build
[![Build Status](https://travis-ci.org/ofats/any_invocable.svg?branch=master)](https://travis-ci.org/ofats/any_invocable)

### Motivating Example
Such a code:
```c++
// requires: C++14
#include <functional>
#include <memory>

struct widget {
    // ...
};

int main() {
    std::function<void()> f = [w_ptr = std::make_unique<widget>()] {
        ...
    };
}
```
Will result in:
```
error: use of deleted function ‘std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&) [with _Tp = widget; _Dp = std::default_delete<widget>]
```
In other words, we can't use std::function to store move-only callbacks or similar stuff.

At the same time such a code will work:
```c++
std::any_invocable<void()> f = [w_ptr = std::make_unique<widget>()] {
    ...
};
```
