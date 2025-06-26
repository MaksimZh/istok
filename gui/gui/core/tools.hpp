// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

template <typename T>
struct Position {
    T x;
    T y;
};


template <typename T>
struct Size {
    T width;
    T height;
};


template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};
