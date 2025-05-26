// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a developer
    I want to manage windows as widgets on a virtual screen root widget
    to use same approaches for window and widget management
    and easily migrate on platforms without windows (like Android)
*/

#include <vector>

using namespace std;


template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};


class Context final {
public:
    Context(const vector<Rect<int>>& monitorLocations) {}

    bool screen_area_is(Rect<int> area) {
        return true;
    }
};


TEST_CASE("Init on one monitor", "[e2e][gui]") {
    // Given
    vector<Rect<int>> monitorLocations{
        Rect<int>{0, 0, 640, 480}
    };
    // When empty screen created
    Context ctx(monitorLocations);
    // Then
    REQUIRE(ctx.screen_area_is(Rect<int>{0, 0, 640, 480}));
}


TEST_CASE("Init on two monitors", "[e2e][gui]") {
    // Given
    vector<Rect<int>> monitorLocations{
        Rect<int>{0, 0, 640, 480},
        Rect<int>{-640, 0, 0, 480}
    };
    // When empty screen created
    Context ctx(monitorLocations);
    // Then
    REQUIRE(ctx.screen_area_is(Rect<int>{-640, 0, 640, 480}));
}
