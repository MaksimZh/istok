// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once


namespace Istok::GUI::GL {

/** @brief Interface for RAII wrapper of current OpenGL context control
 *  
 * All functions that require active OpenGL context must require reference
 * to CurrentGL as argument. This ensures that all wrappers around OpenGL
 * will be used within correct state.
 */
class CurrentGL {
public:
    CurrentGL() = default;
    virtual ~CurrentGL() = 0;
    CurrentGL(const CurrentGL&) = delete;
    CurrentGL& operator=(const CurrentGL&) = delete;
    CurrentGL(CurrentGL&&) = delete;
    CurrentGL& operator=(CurrentGL&&) = delete;
};

CurrentGL::~CurrentGL() = default;

} // namespace Istok::GUI::GL
