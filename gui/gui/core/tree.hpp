// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <list>


template <typename T>
class NodeContainer {
public:
    NodeContainer() = default;

    void push_back(T& node) {
        assert(!contains(node));
        items.push_back(&node);
    }
    
    void erase(T& node) {
        auto it = std::find(items.begin(), items.end(), &node);
        assert(it != items.end());
        items.erase(it);
    }
    
    bool contains(const T& node) {
        return std::find(items.begin(), items.end(), &node) != items.end();
    }

private:
    std::list<T*> items;
};


template <typename T>
class NodeFilter {
public:
    NodeFilter() = default;

    void insert(T& node) {}
    void erase(T& node) {}

    bool contains(const T& node) {
        return false;
    }

    class Range {
        class Iterator {};
    };

    Range filter(NodeContainer<T> source) {
        return Range{};
    }
};


template <typename T>
class Node {
public:
private:
    NodeContainer<T> layoutChildrenSequence;
    NodeContainer<T> renderChildrenSequence;
    NodeFilter<T> visibleChildren;
};
