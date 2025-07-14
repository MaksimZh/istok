// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <queue>

namespace Istok {
namespace ECS {

template <typename T>
class Queue {
public:
    bool empty() const {
        return container.empty();
    }

    void push(T value) {
        container.push(value);
    }

    T& front() {
        return container.front();
    }

    void pop() {
        container.pop();
    }

private:
    std::queue<T> container;
};

} // namespace ECS
} // namespace Istok
