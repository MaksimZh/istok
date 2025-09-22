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
    using Subscriber = std::function<void(const T&)>;
    virtual ~Broadcaster() = default;
    virtual void subscribe(Subscriber subscriber) = 0;
};

template <typename T>
class ConsumerChain {
public:
    using Consumer = std::function<std::optional<T>(T&&)>;
    virtual ~ConsumerChain() = default;
    virtual void chainConsumer(Consumer consumer) = 0;
};

template <typename R, typename T>
class ProcessorChain {
public:
    using Processor = std::function<std::optional<R>(const T&)>;
    virtual ~ProcessorChain() = default;
    virtual void chainProcessor(Processor processor) = 0;
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


template <typename T>
class ConsumingDispatcher: public ConsumerChain<T> {
public:
    using Consumer = typename ConsumerChain<T>::Consumer;
    
    ConsumingDispatcher() = default;
    ConsumingDispatcher(const ConsumingDispatcher&) = delete;
    ConsumingDispatcher& operator=(const ConsumingDispatcher&) = delete;
    ConsumingDispatcher(ConsumingDispatcher&&) = default;
    ConsumingDispatcher& operator=(ConsumingDispatcher&&) = default;

    void chainConsumer(Consumer consumer) override {
        consumers.push_back(consumer);
    }

    std::optional<T> operator()(T&& x) const {
        return std::nullopt;
    }

private:
    std::vector<Consumer> consumers;
};


template <typename R, typename T>
class ReturningDispatcher: public ProcessorChain<R, T> {
public:
    using Processor = typename ProcessorChain<R, T>::Processor;
    
    ReturningDispatcher() = default;
    ReturningDispatcher(const ReturningDispatcher&) = delete;
    ReturningDispatcher& operator=(const ReturningDispatcher&) = delete;
    ReturningDispatcher(ReturningDispatcher&&) = default;
    ReturningDispatcher& operator=(ReturningDispatcher&&) = default;

    void chainProcessor(Processor processor) override {
        processors.push_back(processor);
    }

    std::optional<R> operator()(const T& x) const {
        for (auto& h : processors) {
            if (auto r = h(x)) {
                return r;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<Processor> processors;
};


template <typename T>
class MessageBus: public Sink<T>, public Broadcaster<T> {
public:
    using Subscriber = typename Broadcaster<T>::Subscriber;

    MessageBus() = default;
    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;
    MessageBus(MessageBus&&) = default;
    MessageBus& operator=(MessageBus&&) = default;
    
    void push(T&& value) override {
        queue.push(std::move(value));
        while (true) {
            auto optMessage = queue.take();
            if (!optMessage) {
                break;
            }
            for (auto& h : subscribers) {
                h(optMessage.value());
            }
        }
    }

    void subscribe(Subscriber subscriber) override {
        subscribers.push_back(subscriber);
    }

private:
    Queue<T> queue;
    std::vector<Subscriber> subscribers;
};


} // namespace Istok::Tools
