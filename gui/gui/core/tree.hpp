// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <list>
#include <unordered_set>


template <typename T>
class NodeFilter {
public:
    NodeFilter() = default;

    void insert(T& node) {
        items.insert(&node);
    }
    
    void erase(T& node) {
        assert(contains(node));
        items.erase(&node);
    }

    bool contains(const T& node) {
        return items.contains(const_cast<T*>(&node));
    }

private:
    std::unordered_set<T*> items;
};


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

    class Range {
    public:
        Range(
            NodeFilter<T>* filter,
            std::list<T*>::iterator start,
            std::list<T*>::iterator stop
        ) : filter(filter), start(start), stop(stop) {}

        class Iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;

            Iterator() {}
            Iterator(
                NodeFilter<T>* filter,
                std::list<T*>::iterator current,
                std::list<T*>::iterator stop
            ) : filter(filter), current(current), stop(stop) {
                advance();
            }
            
            T& operator*() const {
                return **current;
            }
            
            Iterator& operator++() {
                ++current;
                advance();
                return *this;
            }
            
            Iterator operator++(int) {
                auto tmp = *this;
                ++*this;
                return tmp;
            }
            
            bool operator==(const Iterator& other) const {
                return current == other.current;
            }
        
        private:
            NodeFilter<T>* filter = nullptr;
            std::list<T*>::iterator current;
            std::list<T*>::iterator stop;

            void advance() {
                if (!filter) {
                    return;
                }
                while (current != stop && !filter->contains(**current)) {
                    ++current;
                }
            }
        };
    
        Iterator begin() {
            return Iterator(filter, start, stop);
        }

        Iterator end() {
            return Iterator(filter, stop, stop);
        }

    private:
        NodeFilter<T>* filter;
        std::list<T*>::iterator start;
        std::list<T*>::iterator stop;
    };

    Range filter(NodeFilter<T>& filter) {
        return Range(&filter, items.begin(), items.end());
    }

    Range getAll() {
        return Range(nullptr, items.begin(), items.end());
    }

private:
    std::list<T*> items;
};


template <typename T>
class Node {
public:
    T* getParent() {
        return parent;
    }

    NodeContainer<T>::Range getChildren() {
        return children.getAll();
    }

    NodeContainer<T>::Range getVisibleChildren() {
        return children.filter(visibleChildren);
    }

    void addChild(T& node) {
        assert(node.getParent() == nullptr);
        assert(!children.contains(node));
        node.parent = self();
        children.push_back(node);
        visibleChildren.insert(node);
    }

    void removeChild(T& node) {
        assert(node.getParent() == self());
        assert(children.contains(node));
        node.parent = nullptr;
        children.erase(node);
        visibleChildren.erase(node);
    }

private:
    T* parent = nullptr;
    NodeContainer<T> children;
    NodeFilter<T> visibleChildren;

    T* self() {
        return static_cast<T*>(this);
    }
};
