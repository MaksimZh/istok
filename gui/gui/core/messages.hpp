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
    SimpleQueue(SimpleQueue&&) = default;
    SimpleQueue& operator=(SimpleQueue&&) = default;

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
class SyncWaitingQueue {
public:
    SyncWaitingQueue() = default;
    SyncWaitingQueue(const SyncWaitingQueue&) = delete;
    SyncWaitingQueue& operator=(const SyncWaitingQueue&) = delete;
    SyncWaitingQueue(SyncWaitingQueue&&) = delete;
    SyncWaitingQueue& operator=(SyncWaitingQueue&&) = delete;

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


template <typename T, typename Notifier>
class NotifyingQueue {
public:
    NotifyingQueue(const NotifyingQueue&) = delete;
    NotifyingQueue& operator=(const NotifyingQueue&) = delete;
    NotifyingQueue(NotifyingQueue&&) = delete;
    NotifyingQueue& operator=(NotifyingQueue&&) = delete;

    NotifyingQueue(Notifier&& notifier) : notifier(std::move(notifier)) {}
    
    bool empty() const {
        return container.empty();
    }

    void push(T&& value) {
        container.push(std::move(value));
        notifier();
    }

    T take() {
        assert(!container.empty());
        return container.take();
    }

private:
    Notifier notifier;
    SimpleQueue<T> container;
};


template <typename T, typename Notifier>
class LazyNotifyingQueue {
private:
    using Target = NotifyingQueue<T, Notifier>;
public:
    LazyNotifyingQueue() = default;
    LazyNotifyingQueue(const LazyNotifyingQueue&) = delete;
    LazyNotifyingQueue& operator=(const LazyNotifyingQueue&) = delete;
    LazyNotifyingQueue(LazyNotifyingQueue&&) = delete;
    LazyNotifyingQueue& operator=(LazyNotifyingQueue&&) = delete;

    void setNotifier(Notifier&& notifier) {
        assert(!target);
        target = std::make_unique<Target>(std::move(notifier));
        while (!buffer.empty()) {
            target->push(buffer.take());
        }
    }

    bool empty() const {
        if (!target) {
            return buffer.empty();
        }
        return target->empty();
    }

    void push(T&& value) {
        if (target) {
            target->push(std::move(value));
            return;
        }
        buffer.push(std::move(value));
    }

    T take() {
        assert(target);
        return target->take();
    }

private:
    std::unique_ptr<Target> target;
    SimpleQueue<T> buffer;
};


template <typename T, typename Notifier>
class SyncLazyNotifyingQueue {
public:
    SyncLazyNotifyingQueue() = default;
    SyncLazyNotifyingQueue(const SyncLazyNotifyingQueue&) = delete;
    SyncLazyNotifyingQueue& operator=(const SyncLazyNotifyingQueue&) = delete;
    SyncLazyNotifyingQueue(SyncLazyNotifyingQueue&&) = delete;
    SyncLazyNotifyingQueue& operator=(SyncLazyNotifyingQueue&&) = delete;

    void setNotifier(Notifier&& notifier) {
        std::lock_guard lock(mut);
        container.setNotifier(std::move(notifier));
    }

    bool empty() {
        std::lock_guard lock(mut);
        return container.empty();
    }

    void push(T&& value) {
        std::lock_guard lock(mut);
        container.push(std::move(value));
    }

    T take() {
        std::lock_guard lock(mut);
        assert(!container.empty());
        return container.take();
    }

private:
    std::mutex mut;
    LazyNotifyingQueue<T, Notifier> container;
};


} // namespace Istok::GUI
