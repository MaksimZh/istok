// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <type_traits>
#include <queue>
#include <mutex>
#include <optional>


namespace Istok::Tools {

template <typename T>
class SimpleQueue {
public:
    SimpleQueue() = default;
    SimpleQueue(const SimpleQueue&) = delete;
    SimpleQueue& operator=(const SimpleQueue&) = delete;
    SimpleQueue(SimpleQueue&&) = default;
    SimpleQueue& operator=(SimpleQueue&&) = default;

    bool empty() const noexcept {
        return container.empty();
    }

    void push(T&& value) noexcept {
        container.push(std::move(value));
    }

    void push(const T& value) noexcept {
        container.push(value);
    }

    T take() {
        if (container.empty()) {
            throw std::runtime_error("Reading from empty queue");
        }
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

    bool empty() const noexcept {
        return container.empty();
    }

    void push(T&& value) noexcept {
        container.push(std::move(value));
        cv.notify_one();
    }

    void push(const T& value) noexcept {
        container.push(value);
        cv.notify_one();
    }

    T take(std::unique_lock<std::mutex>& lock) noexcept {
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

    bool empty() noexcept {
        std::lock_guard lock(mut);
        return container.empty();
    }

    void push(T&& value) noexcept {
        std::lock_guard lock(mut);
        container.push(std::move(value));
    }

    void push(const T& value) noexcept {
        std::lock_guard lock(mut);
        container.push(value);
    }

    T take() noexcept {
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
    
    bool empty() const noexcept {
        return container.empty();
    }

    void push(T&& value) noexcept {
        container.push(std::move(value));
        notifier();
    }

    void push(const T& value) noexcept {
        container.push(value);
        notifier();
    }

    std::optional<T> take() noexcept {
        if (container.empty()) {
            return std::nullopt;
        }
        return container.take();
    }

private:
    Notifier notifier;
    SimpleQueue<T> container;
};


template <typename T, typename Notifier>
class SyncNotifyingQueue {
public:
    SyncNotifyingQueue(const SyncNotifyingQueue&) = delete;
    SyncNotifyingQueue& operator=(const SyncNotifyingQueue&) = delete;
    SyncNotifyingQueue(SyncNotifyingQueue&&) = delete;
    SyncNotifyingQueue& operator=(SyncNotifyingQueue&&) = delete;

    SyncNotifyingQueue(Notifier&& notifier)
        : container(std::move(notifier)) {}

    bool empty() noexcept {
        std::lock_guard lock(mut);
        return container.empty();
    }

    void push(T&& value) noexcept {
        std::lock_guard lock(mut);
        container.push(std::move(value));
    }

    void push(const T& value) noexcept {
        std::lock_guard lock(mut);
        container.push(value);
    }

    std::optional<T> take() noexcept {
        std::lock_guard lock(mut);
        return container.take();
    }

private:
    std::mutex mut;
    NotifyingQueue<T, Notifier> container;
};

} // namespace Istok::Tools
