// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <stdexcept>


template <typename T, typename A>
using MethodPtr = void (T::*)(A&);


template <typename A, typename K>
class Dispatcher {
public:
    void operator()(A& arg) {
        if (!callerMap) {
            throw std::runtime_error("Dispatcher not initialized");
        }
        (*callerMap)(*this, arg);
    }

protected:
    virtual K keyof(const A& arg) const = 0;
    
    class Caller {
    public:
        virtual K key() const = 0;
        virtual bool fits(const A& arg) const = 0;
        virtual void operator()(Dispatcher& handler, A& arg) = 0;
    };

    
    class CallerMap {
    public:
        CallerMap() = default;
        CallerMap(std::vector<std::unique_ptr<Caller>> arg)
        : callers(std::move(arg)) {
            for (auto& c : callers) {
                caller_map[c->key()] = c.get();
            }
        }

        void operator()(Dispatcher& handler, A& arg) {
            K key = handler.keyof(arg);
            if (caller_map.contains(key)) {
                (*caller_map[key])(handler, arg);
                return;
            }
            for (auto& caller : callers) {
                if (caller->fits(arg)) {
                    caller_map[key] = caller.get();
                    (*caller)(handler, arg);
                    return;
                }
            }
            throw std::runtime_error("Caller not found");
        }
    
    private:
        std::vector<std::unique_ptr<Caller>> callers;
        std::unordered_map<K, Caller*> caller_map;
    };

    
    bool initialized() const {
        return callerMaps.contains(std::type_index(typeid(*this)));
    }

    void firstInit(std::vector<std::unique_ptr<Caller>> callers) {
        std::type_index index(typeid(*this));
        callerMaps[index] = CallerMap(std::move(callers));
        callerMap = &callerMaps[index];
    }

    void nextInit() {
        std::type_index index(typeid(*this));
        callerMap = &callerMaps[index];
    }

private:
    static std::unordered_map<std::type_index, CallerMap> callerMaps;
    CallerMap* callerMap = nullptr;
};

template <typename A, typename K>
std::unordered_map<
        std::type_index,
        typename Dispatcher<A, K>::CallerMap>
    Dispatcher<A, K>::callerMaps;
