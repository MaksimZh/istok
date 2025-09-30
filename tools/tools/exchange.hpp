// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>
#include <queue>
#include <optional>
#include <vector>
#include <memory>

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
using Acceptor = std::function<void(const T&)>;

template <typename T>
class AcceptorChain {
public:
    AcceptorChain() = default;
    AcceptorChain(const AcceptorChain&) = delete;
    AcceptorChain& operator=(const AcceptorChain&) = delete;
    AcceptorChain(AcceptorChain&&) = default;
    AcceptorChain& operator=(AcceptorChain&&) = default;

    void chainAcceptor(Acceptor<T> acceptor) {
        acceptors.push_back(acceptor);
    }

    void operator()(const T& x) const {
        for (auto& f : acceptors) {
            f(x);
        }
    }

private:
    std::vector<Acceptor<T>> acceptors;
};


template <typename T>
using Consumer = std::function<std::optional<T>(T&&)>;

template <typename T>
class ConsumerChain {
public:
    ConsumerChain() = default;
    ConsumerChain(const ConsumerChain&) = delete;
    ConsumerChain& operator=(const ConsumerChain&) = delete;
    ConsumerChain(ConsumerChain&&) = default;
    ConsumerChain& operator=(ConsumerChain&&) = default;

    void chainConsumer(Consumer<T> consumer) {
        consumers.push_back(consumer);
    }

    std::optional<T> dispatch(T&& x) const {
        std::optional<T> optX = std::move(x);
        for (auto& f : consumers) {
            if (!optX) {
                break;
            }
            optX = f(std::move(optX.value()));
        }
        return std::move(optX);
    }

private:
    std::vector<Consumer<T>> consumers;
};


template <typename R, typename T>
using Processor = std::function<std::optional<R>(const T&)>;

template <typename R, typename T>
class ProcessorChain {
public:
    ProcessorChain() = default;
    ProcessorChain(const ProcessorChain&) = delete;
    ProcessorChain& operator=(const ProcessorChain&) = delete;
    ProcessorChain(ProcessorChain&&) = default;
    ProcessorChain& operator=(ProcessorChain&&) = default;

    void chainProcessor(Processor<R, T> processor) {
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
    std::vector<Processor<R, T>> processors;
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
