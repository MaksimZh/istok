// messages.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <queue>
#include <mutex>


namespace Istok::GUI {

template <typename T>
class SimpleQueue {
public:
    SimpleQueue() = default;
    SimpleQueue(const SimpleQueue&) = delete;
    SimpleQueue& operator=(const SimpleQueue&) = delete;
    SimpleQueue(SimpleQueue&&) = delete;
    SimpleQueue& operator=(SimpleQueue&&) = delete;

    bool empty() const {
        return container.empty();
    }

    void push(T&& value) {
        container.push(std::move(value));
    }

    T take() {
        assert(!container.empty());
        T value = std::move(container.front());
        container.pop();
        return value;
    }

private:
    std::queue<T> container;
};


template <typename T>
class WaitingQueue {
public:
    WaitingQueue() = default;
    WaitingQueue(const WaitingQueue&) = delete;
    WaitingQueue& operator=(const WaitingQueue&) = delete;
    WaitingQueue(WaitingQueue&&) = delete;
    WaitingQueue& operator=(WaitingQueue&&) = delete;

    bool empty() const {
        return container.empty();
    }

    void push(T&& value) {
        container.push(std::move(value));
        cv.notify_one();
    }

    T take(std::unique_lock<std::mutex>& lock) {
        cv.wait(lock, [&]{ return !empty(); });
        assert(!container.empty());
        return container.take();
    }

private:
    SimpleQueue<T> container;
    std::condition_variable cv;
};


template <typename T>
class SyncQueue {
public:
    SyncQueue() = default;
    SyncQueue(const SyncQueue&) = delete;
    SyncQueue& operator=(const SyncQueue&) = delete;
    SyncQueue(SyncQueue&&) = delete;
    SyncQueue& operator=(SyncQueue&&) = delete;

    bool empty() {
        std::lock_guard lock(mut);
        return container.empty();
    }

    void push(T&& value) {
        std::lock_guard lock(mut);
        container.push(std::move(value));
    }

    T take() {
        std::unique_lock lock(mut);
        return container.take(lock);
    }

private:
    std::mutex mut;
    WaitingQueue<T> container;
};

} // namespace Istok::GUI
