// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

template <typename D, typename T>
using MethodPtr = void (D::*)(T&);


template <typename T, typename KF>
class Dispatcher {
public:
    class Caller {
    public:
        virtual void operator()(Dispatcher& dispatcher, T& target) = 0;
        virtual bool fits(T& target) = 0;
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

    Dispatcher(KF keyFunc) {}

    void operator()(T& arg) {
        (*method)(*this, arg);
    }

protected:
    template <typename C>
    void init(const C& caller) {
        this->method = std::make_unique<C>(caller);
    }

private:
    std::unique_ptr<Caller> method;
};
