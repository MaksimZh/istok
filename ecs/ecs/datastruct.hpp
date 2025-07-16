// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <queue>

namespace Istok {
namespace ECS {

template <typename T>
class Queue {
public:
    Queue() = default;
    Queue(const Queue& other) = delete;
    Queue& operator=(const Queue& other) = delete;
    Queue(Queue&& other) noexcept = default;
    Queue& operator=(Queue&& other) noexcept = default;
    
    bool empty() const {
        return container.empty();
    }

    void push(const T& value) {
        container.push(value);
    }

    void push(T&& value) {
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


template <typename T>
class DenseRange {
public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    DenseRange(std::vector<T>& data) : data(data) {}

    iterator begin() noexcept { return data.begin(); }
    iterator end() noexcept { return data.end(); }
    const_iterator begin() const noexcept { return data.cbegin(); }
    const_iterator end() const noexcept { return data.cend(); }

private:
    std::vector<T>& data;
};


template <typename T>
class DenseArray {
public:
    DenseArray() = default;
    DenseArray(const DenseArray& other) = delete;
    DenseArray& operator=(const DenseArray& other) = delete;
    DenseArray(DenseArray&& other) noexcept = default;
    DenseArray& operator=(DenseArray&& other) noexcept = default;

    size_t size() const noexcept {
        return container.size();
    }
    
    T& operator[](size_t index) {
        assert(index < size());
        return container[index];
    }
    
    const T& operator[](size_t index) const {
        assert(index < size());
        return container[index];
    }

    void push_back(const T& value) {
        container.push_back(value);
    }

    void push_back(T&& value) {
        container.push_back(std::move(value));
    }

    void erase(size_t index) {
        assert(index <= size());
        if (index < size() - 1) {
            container[index] = std::move(container.back());
        }
        container.pop_back();
    }

    DenseRange<T> byElement() noexcept {
        return DenseRange<T>(container);
    }

    DenseRange<T> byElement() const noexcept {
        return DenseRange<T>(const_cast<const std::vector<T>&>(container));
    }

private:
    std::vector<T> container;
};

} // namespace ECS
} // namespace Istok
