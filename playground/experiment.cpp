#include "logging.hpp"
#include <iostream>
#include <internal/component.hpp>

using namespace Istok::ECS;

struct A {
    int value;
    bool operator==(const A&) const = default;
};

struct B {
    int value;
    bool operator==(const B&) const = default;
};

struct C {
    int value;
    bool operator==(const C&) const = default;
};

int main() {
    SET_LOGGER("ecs", Istok::Logging::TerminalLogger::GetInstance(), Istok::Logging::Level::all);
    ComponentManager cm;
    cm.insert(0, A{0});
    cm.insert(1, A{0});
    cm.insert(2, A{0});
    cm.insert(3, A{0});
    cm.insert(2, B{0});
    cm.insert(3, B{0});
    cm.insert(4, B{0});
    cm.insert(5, B{0});
    cm.insert(0, C{0});
    cm.insert(2, C{0});
    cm.insert(4, C{0});
    cm.insert(6, C{0});
    for (const auto& i : cm.view<A, B>()) {
        std::cout << i << std::endl;
    }
    return 0;
}
