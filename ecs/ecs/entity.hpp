// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <queue>


class LimitedCounter {
public:
    LimitedCounter(size_t value, size_t sentinel)
        : value(value), sentinel(sentinel) {}
    
    bool isFull() const {
        return value == sentinel;
    }

    void extendBy(size_t shift) {
        sentinel += shift;
    }

    operator size_t() const {
        return value;
    }

    LimitedCounter& operator++() {
        assert(!isFull());
        ++value;
        return *this;
    }

    LimitedCounter operator++(int) {
        assert(!isFull());
        auto tmp = *this;
        ++*this;
        return tmp;
    }

private:
    size_t value;
    size_t sentinel;
};


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
