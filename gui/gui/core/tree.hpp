// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <iterator>
#include <ranges>


template <typename T, typename M>
class NodeIterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;

    NodeIterator(T* current = nullptr) : current(current) {}

    T& operator*() const { return *current; }

    NodeIterator& operator++() {
        current = current.getNext(M::instance);
        return *this;
    }
    
    NodeIterator operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const NodeIterator& other) const {
        return current == other.current;
    }

private:
    T* current = nullptr;
};


template <typename T, typename M>
class NodeRange {
public:
    NodeRange(T* head) : head(head) {}

    NodeIterator<T, M> begin() {
        return NodeIterator<T, M>(head);
    }
    
    NodeIterator<T, M> end() {
        return NodeIterator<T, M>(nullptr);
    }

private:
    T* head;
};


template <typename T, typename M>
class Node {
public:
    T* getParent() {
        return parent;
    }

    NodeRange<T, M> getChildren(const M& marker) {
        return NodeRange<T, M>(nullptr);
    }

private:
    T* parent = nullptr;
};
