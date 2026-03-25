// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>


namespace Istok::ECS {

using Closure = std::move_only_function<void() noexcept>;

namespace Internal {

class ClosureLoop {
public:
    ClosureLoop() = default;

    ~ClosureLoop() {
        while (!closures_.empty()) {
            closures_.pop_back();
        }
    }

    ClosureLoop(const ClosureLoop&) = delete;
    ClosureLoop& operator=(const ClosureLoop&) = delete;
    ClosureLoop(ClosureLoop&&) = default;
    ClosureLoop& operator=(ClosureLoop&&) = default;

    void add(Closure&& closure) noexcept {
        closures_.push_back(std::move(closure));
    }

    void iterate() noexcept {
        if (state_ != kIdleState) {
            return;
        }
        for (state_ = 0; state_ < closures_.size(); ++state_) {
            closures_[state_]();
        }
        state_ = -1;
    }

    void pass() noexcept {
        if (state_ < 0) {
            return;
        }
        size_t sentinel = state_;
        state_ = kPassState;
        for (size_t i = sentinel + 1; i < closures_.size(); ++i) {
            closures_[i]();
        }
        for (size_t i = 0; i < sentinel; ++i) {
            closures_[i]();
        }
        state_ = sentinel;
    }

private:
    constexpr static int kIdleState = -1;
    constexpr static int kPassState = -2;
    std::vector<Closure> closures_;
    int state_ = kIdleState;
};

}  // namespace Internal

}  // namespace Istok::ECS
