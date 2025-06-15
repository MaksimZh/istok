// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

template <typename D, typename T>
using MethodPtr = void (D::*)(T&);


template <typename D, typename T>
class Caller {
public:
    virtual void operator()(D& dispatcher, T& target) = 0;
    virtual bool fits(T& target) = 0;
};


template <typename D, typename T, typename D1>
class SubCaller : public Caller<D, T> {
public:
    SubCaller(MethodPtr<D1, T> method) : method(method) {}

    void operator()(D& dispatcher, T& target) override {
        (static_cast<D1*>(&dispatcher)->*method)(target);
    }

private:
    MethodPtr<D1, T> method;
};


template <typename T, typename KF>
class Dispatcher {
public:
    Dispatcher(KF keyFunc, std::unique_ptr<Caller<Dispatcher, T>> method)
        : method(std::move(method)) {}

    void operator()(T& arg) {
        (*method)(*this, arg);
    }

private:
    std::unique_ptr<Caller<Dispatcher, T>> method;
};
