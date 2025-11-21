// manager.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>
#include <cassert>
#include <stdexcept>

namespace Istok::ECS {

class System {
public:
    virtual ~System() = default;
    virtual void run() = 0;
};


class SystemStack {
public:
    SystemStack() = default;
    
    SystemStack(const SystemStack&) = delete;
    SystemStack& operator=(const SystemStack&) = delete;
    SystemStack(SystemStack&&) = delete;
    SystemStack& operator=(SystemStack&&) = delete;

    ~SystemStack() {
        while (!empty()) {
            pop();
        }
    }

    bool empty() const {
        return systems_.empty();
    }

    void push(std::unique_ptr<System>&& system) {
        if (!system) {
            throw std::runtime_error("Null system pointer");
        }
        systems_.push_back(std::move(system));
    }

    void pop() {
        if (systems_.empty()) {
            throw std::runtime_error("No system to pop");
        }
        systems_.pop_back();
    }

    void run() {
        for (auto& system : systems_) {
            assert(system);
            system->run();
        }
    }

private:
    std::vector<std::unique_ptr<System>> systems_;
};


} // namespace Istok::ECS
