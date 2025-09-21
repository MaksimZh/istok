// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>
#include <queue>
#include <optional>

namespace Istok::Tools {

template <typename T>
class Sink {
public:
    virtual ~Sink() = default;
    virtual void push(T&& value) = 0;
};

template <typename T>
class Source {
public:
    virtual ~Source() = default;
    virtual T take() = 0;
};

template <typename T>
class Broadcaster {
public:
    virtual ~Broadcaster() = default;
    virtual void subscribe(std::function<void(const T&)> callback) = 0;
};


template <typename T>
class Queue: public Sink<T>, public Source<std::optional<T>> {
public:
    Queue() = default;
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue(Queue&&) = default;
    Queue& operator=(Queue&&) = default;

    void push(T&& value) override {
        container.push(std::move(value));
    }

    std::optional<T> take() override {
        if (container.empty()) {
            return std::nullopt;
        }
        T value = std::move(container.front());
        container.pop();
        return value;
    }

private:
    std::queue<T> container;
};


} // namespace Istok::Tools
