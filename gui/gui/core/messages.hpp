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


class QueueMutex {
public:
    class check_lock {
    public:
        check_lock() = delete;
        check_lock(const check_lock&) = delete;
        check_lock& operator=(const check_lock&) = delete;
        check_lock(check_lock&&) = delete;
        check_lock& operator=(check_lock&&) = delete;

        check_lock(QueueMutex& qm) : lock(qm.mut) {}
    
    private:
        std::lock_guard<std::mutex> lock;
    };

    class write_lock {
    public:
        write_lock() = delete;
        write_lock(const write_lock&) = delete;
        write_lock& operator=(const write_lock&) = delete;
        write_lock(write_lock&&) = delete;
        write_lock& operator=(write_lock&&) = delete;

        write_lock(QueueMutex& qm) : lock(qm.mut), cv(qm.cv) {}
        
        ~write_lock() {
            cv.notify_one();
        }
    
    private:
        std::lock_guard<std::mutex> lock;
        std::condition_variable& cv;
    };

    class read_lock {
    public:
        read_lock() = delete;
        read_lock(const read_lock&) = delete;
        read_lock& operator=(const read_lock&) = delete;
        read_lock(read_lock&&) = delete;
        read_lock& operator=(read_lock&&) = delete;

        template <typename Q>
        read_lock(QueueMutex& qm, Q& container) : lock(qm.mut) {
            qm.cv.wait(lock, [&container]{ return !container.empty(); });
        }
    
    private:
        std::unique_lock<std::mutex> lock;
    };

private:
    std::mutex mut;
    std::condition_variable cv;
};


template <typename T>
class SyncQueue {
public:
    using value_type = T;
    
    SyncQueue() = default;
    SyncQueue(const SyncQueue&) = delete;
    SyncQueue& operator=(const SyncQueue&) = delete;
    SyncQueue(SyncQueue&&) = default;
    SyncQueue& operator=(SyncQueue&&) = default;

    bool empty() {
        QueueMutex::check_lock lock(mut);
        return container.empty();
    }

    void push(T&& value) {
        QueueMutex::write_lock lock(mut);
        container.push(std::move(value));
    }

    T take() {
        QueueMutex::read_lock lock(mut, container);
        assert(!container.empty());
        return container.take();
    }

private:
    QueueMutex mut;
    SimpleQueue<T> container;
};


template <typename Q>
class MessageDest;

template <typename Q>
class MessageSource;

template <typename Q, typename... Args>
std::pair<MessageDest<Q>, MessageSource<Q>> makeMessageQueue(Args&&... args) {
    auto container = std::make_shared<Q>(args...);
    return std::make_pair(
        MessageDest<Q>(container),
        MessageSource<Q>(container));
}


template <typename Q>
class MessageDest {
public:
    MessageDest() = delete;
    MessageDest(const MessageDest&) = delete;
    MessageDest& operator=(const MessageDest&) = delete;
    MessageDest(MessageDest&&) = default;
    MessageDest& operator=(MessageDest&&) = default;
    
    void push(Q::value_type&& value) {
        container->push(std::move(value));
    }

private:
    std::shared_ptr<Q> container;

    MessageDest(std::shared_ptr<Q> container) : container(container) {}
    
    template <typename U, typename... Args>
    friend std::pair<MessageDest<U>, MessageSource<U>>
        makeMessageQueue(Args&&... args);
};


template <typename Q>
class MessageSource {
public:
    MessageSource() = delete;
    MessageSource(const MessageSource&) = delete;
    MessageSource& operator=(const MessageSource&) = delete;
    MessageSource(MessageSource&&) = default;
    MessageSource& operator=(MessageSource&&) = default;
    
    bool empty() { return container->empty(); }
    Q::value_type take() { return container->take(); }

private:
    std::shared_ptr<Q> container;

    MessageSource(std::shared_ptr<Q> container) : container(container) {}

    template <typename U, typename... Args>
    friend std::pair<MessageDest<U>, MessageSource<U>>
        makeMessageQueue(Args&&... args);
};


} // namespace Istok::GUI
