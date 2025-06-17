// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

template <typename D, typename T>
using MethodPtr = void (D::*)(T&);


template <typename T, typename KF>
class Dispatcher {
public:
    using KeyType = decltype(std::declval<KF>()(std::declval<T>()));

    class Caller {
    public:
        virtual void operator()(Dispatcher& dispatcher, T& target) = 0;
        virtual bool fits(T& target) = 0;
        virtual KeyType key() = 0;
    };


    template <typename D1>
    class SubCaller : public Caller {
    public:
        SubCaller(MethodPtr<D1, T> method) : method(method) {}

        void operator()(Dispatcher& dispatcher, T& target) override {
            (static_cast<D1*>(&dispatcher)->*method)(target);
        }

    private:
        MethodPtr<D1, T> method;
    };

    
    Dispatcher(KF keyFunc) {
        if (!keyFunction) {
            keyFunc = keyFunction;
        }
    }

    void operator()(T& arg) {
        (*method)(*this, arg);
    }

protected:
    template <typename Head, typename... Tail>
    void init(Head&& head, Tail&&... tail) {
        this->method = std::make_unique<Head>(head);
    }

private:
    static KF keyFunction;

    class CallerPack {
    public:
        CallerPack()
        
        template <typename C>
        void add(C&& caller) {
            callers.push_back(std::make_unique<C>(caller));
            KeyType key = keyFunction()
        }

        void operator()(Dispatcher& dispatcher, T& target) {}

    private:
        std::vector<std::unique_ptr<Caller>> callers;
        std::unordered_map<KeyType, Caller*> caller_map;

        template <typename V1, typename T1>
        void addOne(VisitPtr<V1, T1> method) {
            callers.push_back(std::make_unique<VisitCaller<V, T, V1, T1>>(method));
            std::type_index index = std::type_index(typeid(T1));
            caller_map[index] = callers.back().get();
        }
    };

    std::unique_ptr<Caller> method;
};

template <typename T, typename KF>
static KF Dispatcher<T, KF>::keyFunction = nullptr;
