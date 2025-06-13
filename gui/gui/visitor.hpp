#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

template <typename V, typename T>
using VisitPtr = void (V::*)(T&);


template <typename V, typename T>
class BaseVisitHandler {
public:
    virtual void operator()(V& visitor, T& target) = 0;
};

template <typename V, typename T, typename V1, typename T1>
class VisitHandler : public BaseVisitHandler<V, T> {
public:
    VisitHandler(VisitPtr<V1, T1> method) : method(method) {}

    void operator()(V& visitor, T& target) override {
        (static_cast<V1*>(&visitor)->*method)(*static_cast<T1*>(&target));
    }

private:
    VisitPtr<V1, T1> method;
};

/*
template <typename V, typename T>
using HandlerPtr = void (V::*)(T&);

template <typename V, typename T>
class TargetTable {
public:
    template <typename V1, typename T1>
    void add(HandlerPtr<V1, T1> handler) {

    }

private:
    std::unordered_map<std::type_index, HandlerPtr<V, T>> handlers;
};
*/

template <typename T>
class Visitor {
public:
    void visit(T& target) {
        (*handler)(*this, target);
    }

protected:
    template <typename V1, typename T1>
    void registerHandler(VisitPtr<V1, T1> method) {
        handler = std::make_unique<VisitHandler<Visitor, T, V1, T1>>(method);
    }

    static std::unique_ptr<BaseVisitHandler<Visitor, T>> handler;
};

template <typename T>
std::unique_ptr<BaseVisitHandler<Visitor<T>, T>> Visitor<T>::handler;
