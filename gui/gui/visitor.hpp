#pragma once

template <typename T>
class Visitor {
public:
    void visit(T& target) {
        (this->*method)(target);
    }

protected:
    using HandlerPtr = void (Visitor::*)(T&);
    static HandlerPtr method;
};


template <typename T>
Visitor<T>::HandlerPtr Visitor<T>::method = nullptr;
