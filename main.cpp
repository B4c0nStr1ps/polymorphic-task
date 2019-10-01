#include <iostream>
#include "task.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    task<std::unique_ptr<int>()> f = [_p = std::make_unique<int>(42)]() mutable {
        return move(_p);
    };

    task<int(int)> square = [](int i) { return i * i;};

    std::cout << *f() << std::endl;

    std::cout << square(4) << std::endl;

    return 0;
}