#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

template <typename V, typename T>
using VisitPtr = void (V::*)(T&);


template <typename V, typename T>
class BaseVisitCaller {
public:
    virtual void operator()(V& visitor, T& target) = 0;
};

template <typename V, typename T, typename V1, typename T1>
class VisitCaller : public BaseVisitCaller<V, T> {
public:
    VisitCaller(VisitPtr<V1, T1> method) : method(method) {}

    void operator()(V& visitor, T& target) override {
        (static_cast<V1*>(&visitor)->*method)(*static_cast<T1*>(&target));
    }

private:
    VisitPtr<V1, T1> method;
};


template <typename V, typename T>
class Dispatcher {
public:
    Dispatcher() {}

    template <typename... Args>
    void add(Args... args) {
        (addOne(std::forward<Args>(args)), ...);
    }

    void operator()(V& visitor, T& target) {
        std::type_index index = std::type_index(typeid(target));
        if (caller_map.contains(index)) {
            (*caller_map[index])(visitor, target);
            return;
        }
    }


private:
    std::vector<std::unique_ptr<BaseVisitCaller<V, T>>> callers;
    std::unordered_map<std::type_index, BaseVisitCaller<V, T>*> caller_map;

    template <typename V1, typename T1>
    void addOne(VisitPtr<V1, T1> method) {
        callers.push_back(std::make_unique<VisitCaller<V, T, V1, T1>>(method));
        std::type_index index = std::type_index(typeid(T1));
        caller_map[index] = callers.back().get();
    }
};


template <typename T>
class Visitor {
public:
    virtual void visit(T& target) {
        (*dispatcher)(*this, target);
    }

protected:
    template <typename V1, typename T1, typename... Tail>
    void init(VisitPtr<V1, T1> head, Tail... tail) {
        std::type_index index = std::type_index(typeid(V1));
        if (!dispatchers.contains(index)) {
            dispatchers[index] = std::make_unique<Dispatcher<Visitor, T>>();
            dispatchers[index]->add(head, tail...);
        }
        dispatcher = dispatchers[index].get();
    }

private:
    static std::unordered_map<
            std::type_index,
            std::unique_ptr<Dispatcher<Visitor, T>>>
        dispatchers;
    
    Dispatcher<Visitor, T>* dispatcher;
};


template <typename T>
std::unordered_map<
        std::type_index,
        std::unique_ptr<Dispatcher<Visitor<T>, T>>>
    Visitor<T>::dispatchers;
