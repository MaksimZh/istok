#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include <stdexcept>
#include <iostream>

template <typename S, typename T>
using MethodPtr = void (S::*)(T&);


template <typename S, typename T>
class Caller {
public:
    virtual void operator()(S& self, T& target) = 0;
    virtual bool fits(T& target) = 0;
};

template <typename S, typename T, typename S1, typename T1>
class SubCaller : public Caller<S, T> {
public:
    SubCaller(MethodPtr<S1, T1> method) : method(method) {}

    void operator()(S& self, T& target) override {
        (static_cast<S1*>(&self)->*method)(*static_cast<T1*>(&target));
    }

    bool fits(T& target) override {
        return dynamic_cast<T1*>(&target);
    }

private:
    MethodPtr<S1, T1> method;
};


template <typename S, typename T>
class SubDispatcher {
public:
    SubDispatcher() {}

    template <typename... Args>
    void add(Args... args) {
        (addOne(std::forward<Args>(args)), ...);
    }

    void operator()(S& self, T& target) {
        std::type_index index = std::type_index(typeid(target));
        if (caller_map.contains(index)) {
            (*caller_map[index])(self, target);
            return;
        }
        for (auto & caller : callers) {
            if (caller->fits(target)) {
                caller_map[index] = caller.get();
                (*caller)(self, target);
                return;
            }
        }
        throw std::runtime_error("Visitor method not found");
    }


private:
    std::vector<std::unique_ptr<Caller<S, T>>> callers;
    std::unordered_map<std::type_index, Caller<S, T>*> caller_map;

    template <typename S1, typename T1>
    void addOne(MethodPtr<S1, T1> method) {
        callers.push_back(std::make_unique<SubCaller<S, T, S1, T1>>(method));
        std::type_index index = std::type_index(typeid(T1));
        caller_map[index] = callers.back().get();
    }
};


template <typename T>
class Dispatcher {
public:
    virtual void operator()(T& target) {
        (*dispatcher)(*this, target);
    }

protected:
    template <typename S1, typename T1, typename... Tail>
    void init(MethodPtr<S1, T1> head, Tail... tail) {
        std::type_index index = std::type_index(typeid(S1));
        if (!dispatchers.contains(index)) {
            dispatchers[index] = std::make_unique<SubDispatcher<Dispatcher, T>>();
            dispatchers[index]->add(head, tail...);
        }
        dispatcher = dispatchers[index].get();
    }

private:
    static std::unordered_map<
            std::type_index,
            std::unique_ptr<SubDispatcher<Dispatcher, T>>>
        dispatchers;
    
    SubDispatcher<Dispatcher, T>* dispatcher;
};


template <typename T>
std::unordered_map<
        std::type_index,
        std::unique_ptr<SubDispatcher<Dispatcher<T>, T>>>
    Dispatcher<T>::dispatchers;
