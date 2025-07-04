// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <iterator>
#include <ranges>


template <typename T, typename N>
class NodeIterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;

    NodeIterator(T* current = nullptr) : current(current) {}

    T& operator*() const { return *current; }

    NodeIterator& operator++() {
        current = current::N.getNext();
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


template <typename T, typename N>
class NodeRange {
public:
    NodeRange(T* head) : head(head) {}

    NodeIterator<T, N> begin() {
        return NodeIterator<T, N>(head);
    }
    
    NodeIterator<T, N> end() {
        return NodeIterator<T, N>(nullptr);
    }

private:
    T* head;
};


template <typename T, typename N>
class NodeList {
public:
    NodeRange<T, N> getRange() {
        return NodeRange<T, N>(head);
    }
    
    void add(T& node) {
        if (!tail) {
            head = &node;
            tail = &node;
            return;
        }
        tail.N::setNext(&node);
        node.N::setPrev(tail);
        tail = &node;
    }

private:
    T* head = nullptr;
    T* tail = nullptr;
};


template <typename T>
class ChildNode {
public:
    T* getParent() { return parent; }

private:
    T* parent = nullptr;
};


template <typename T, typename N>
class Node: public virtual ChildNode<T> {
public:
    T* getNext() { return next; }
    T* getPrev() { return prev; }
    NodeRange<T, N> getChildren() { return children.getRange(); }

    void addChild(T& node) {
        children.add(node);
    }

private:
    T* next = nullptr;
    T* prev = nullptr;
    NodeList<T, N> children;
};
