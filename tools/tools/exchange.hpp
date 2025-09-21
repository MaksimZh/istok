// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>
#include <queue>
#include <optional>
#include <vector>

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
class ConsumerChain {
public:
    virtual ~ConsumerChain() = default;
    virtual void chainConsumer(std::function<std::optional<T>(T&&)> consumer) = 0;
};

template <typename R, typename T>
class ProcessorChain {
public:
    virtual ~ProcessorChain() = default;
    virtual void chainProcessor(std::function<std::optional<R>(const T&)> processor) = 0;
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


template <typename R, typename T>
class HandlerChain {
public:
    using Handler = std::function<std::optional<R>(const T&)>;
    
    HandlerChain() = default;
    HandlerChain(const HandlerChain&) = delete;
    HandlerChain& operator=(const HandlerChain&) = delete;
    HandlerChain(HandlerChain&&) = default;
    HandlerChain& operator=(HandlerChain&&) = default;

    void add(Handler handler) {
        handlers.push_back(handler);
    }

    std::optional<R> operator()(const T& x) const {
        for (auto& h : handlers) {
            if (auto r = h(x)) {
                return r;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<Handler> handlers;
};


template <typename T>
class MessageBus: public Sink<T>, public Broadcaster<T> {
public:
    using Handler = std::function<void(const T&)>;
    
    void push(T&& value) override {
        queue.push(std::move(value));
        while (true) {
            auto optMessage = queue.take();
            if (!optMessage) {
                break;
            }
            for (auto& h : handlers) {
                h(optMessage.value());
            }
        }
    }

    void subscribe(Handler handler) override {
        handlers.push_back(handler);
    }

private:
    Queue<T> queue;
    std::vector<Handler> handlers;
};


} // namespace Istok::Tools
