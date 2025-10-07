// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <stdexcept>
#include <variant>
#include <optional>
#include <queue>
#include <functional>
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


template<typename A, typename R = void>
class HandlerResult {
    static_assert(!std::is_void_v<R>, "R must not be void in primary template");

    struct Arg {
        A value;
        bool operator==(const Arg&) const = default;
    };

    struct Res {
        R value;
        bool operator==(const Res&) const = default;
    };

    using Data = std::variant<Arg, Res>;
    Data data;
    
    explicit HandlerResult(Data&& value) : data(std::move(value)) {}

public:
    static HandlerResult fromArgument(A&& value) {
        return HandlerResult(Arg(std::move(value)));
    }
    
    static HandlerResult fromResult(R&& value){
        return HandlerResult(Res(std::move(value)));
    }

    bool operator==(const HandlerResult&) const = default;

    bool complete() const noexcept {
        return std::holds_alternative<Res>(data);
    }

    A& argument() {
        if (complete()) {
            throw std::logic_error("HandlerResult: no argument");
        }
        return std::get<Arg>(data).value;
    }

    R& result() {
        if (!complete()) {
            throw std::logic_error("HandlerResult: no result");
        }
        return std::get<Res>(data).value;
    }
};


template<typename A>
class HandlerResult<A, void> {
    std::optional<A> data;

public:
    HandlerResult() : data(std::nullopt) {}
    explicit HandlerResult(A&& value) : data(std::move(value)) {}
    
    static HandlerResult fromArgument(A&& value) {
        return HandlerResult(std::move(value));
    }
    
    bool operator==(const HandlerResult&) const = default;
    
    bool complete() const noexcept {
        return !data.has_value();
    }

    A& argument() {
        if (!data.has_value()) {
            throw std::logic_error("HandlerResult: no argument");
        }
        return data.value();
    }

    void result() const {
        if (data.has_value()) {
            throw std::logic_error("HandlerResult: no result");
        }
    }
};


template <typename A, typename R>
using Handler = std::function<HandlerResult<A, R>(A&&)>;


template <typename A, typename R = void>
class HandlerChain {
public:
    HandlerChain() = default;
    HandlerChain(const HandlerChain&) = delete;
    HandlerChain& operator=(const HandlerChain&) = delete;
    HandlerChain(HandlerChain&&) = default;
    HandlerChain& operator=(HandlerChain&&) = default;

    void append(Handler<A, R> handler) {
        handlers.push_back(handler);
    }

    HandlerResult<A, R> operator()(A&& arg) const {
        auto result = HandlerResult<A, R>::fromArgument(std::move(arg));
        for (auto& h : handlers) {
            if (result.complete()) {
                return result;
            }
            result = h(std::move(result.argument()));
        }
        return result;
    }

private:
    std::vector<Handler<A, R>> handlers;
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

    std::optional<R> dispatch(const T& x) const {
        for (auto& f : processors) {
            if (auto r = f(x)) {
                return r;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<Processor<R, T>> processors;
};


template <typename T>
class MessageBus: public Sink<T> {
public:
    using Subscriber = typename Broadcaster<T>::Subscriber;

    MessageBus() = default;
    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;
    MessageBus(MessageBus&&) = default;
    MessageBus& operator=(MessageBus&&) = default;
    
    void push(T&& value) override {
        queue.push(std::move(value));
        if (!running) {
            processQueue();
        }
    }

    void addSubscriber(Consumer<T> subscriber) {
        dispatcher.chainConsumer(subscriber);
    }

private:
    bool running = false;
    Queue<T> queue;
    ConsumerChain<T> dispatcher;

    void processQueue() {
        running = true;
        while (true) {
            auto optMessage = queue.take();
            if (!optMessage) {
                break;
            }
            dispatcher.dispatch(std::move(optMessage.value()));
        }
        running = false;
    }
};


} // namespace Istok::Tools
