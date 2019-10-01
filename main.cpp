#include <iostream>
#include "task.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    task<std::unique_ptr<int>()> f = [_p = std::make_unique<int>(42)]() mutable {
        return move(_p);
    };

    std::cout << *f() << std::endl;

    return 0;
}