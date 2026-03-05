// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cstdint>
#include <format>
#include <map>
#include <vector>

#include <windows.h>


struct MockWinAPI {
    inline static thread_local std::vector<std::string> log;
    inline static thread_local std::map<HWND, void*> userPointers_;

    static void reset() {
        log.clear();
        userPointers_.clear();
    }

    template<typename T>
    static void setUserPointer(HWND hWnd, T* ptr) noexcept {
        userPointers_[hWnd] = ptr;
    }

    template<typename T>
    static T* getUserPointer(HWND hWnd) noexcept {
        if (auto it = userPointers_.find(hWnd);
            it != userPointers_.end()
        ) {
            return reinterpret_cast<T*>(it->second);
        }
        return nullptr;
    }

    static LRESULT DefWindowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept {
        log.push_back(std::format(
            "DefWindowProc({:#x}, {:#x}, {:#x}, {:#x})",
            reinterpret_cast<uintptr_t>(hWnd), msg, wParam, lParam));
        return 0;
    }
};
