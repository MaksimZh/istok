// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <queue>

namespace ecs {
    template <typename T>
    class Queue {
    public:
        bool empty() const {
            return container.empty();
        }

        void push(T value) {
            container.push(value);
        }

        T pop() {
            T value = container.front();
            container.pop();
            return value;
        }

    private:
        std::queue<T> container;
    };
}
