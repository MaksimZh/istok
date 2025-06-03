// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a developer
    I want to manage windows as widgets on a virtual screen root widget
    to use same approaches for window and widget management
    and easily migrate on platforms without windows (like Android)
*/

#include <vector>
#include <memory>
#include <optional>

using namespace std;


template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};


class WidgetHandler {};

class WindowHandler : public WidgetHandler {};

template <typename T>
struct Limits {
    T lower;
    optional<T> upper;
};

template <typename T>
struct Limits2D {
    Limits<T> x;
    Limits<T> y;
};

struct Widget {
    Limits2D<float> sizing;
    Rect<float> location;
    unique_ptr<WidgetHandler> handler;
};


class Context final {
public:
    Context(const vector<Rect<int>>& monitorLocations) {}

    void create_screen(vector<Widget> children) {}

    bool screen_area_is(Rect<int> area) {
        return true;
    }
};


TEST_CASE("Empty screen on one monitor", "[e2e][gui]") {
    // Given
    Context ctx({Rect<int>{0, 0, 200, 100}});
    // When
    ctx.create_screen({});
    // Then
    REQUIRE(ctx.screen_area_is(Rect<int>{0, 0, 640, 480}));
}


TEST_CASE("Empty screen on two monitors", "[e2e][gui]") {
    // Given
    vector<Rect<int>> monitorLocations{
        Rect<int>{0, 0, 200, 100},
        Rect<int>{-100, -50, 0, 300}
    };
    // When empty screen created
    Context ctx(monitorLocations);
    // Then
    REQUIRE(ctx.screen_area_is(Rect<int>{-100, -50, 200, 300}));
}


TEST_CASE("One window on one monitor", "[e2e][gui]") {
    // Given
    Context ctx({Rect<int>{0, 0, 200, 100}});
    // When
    ctx.create_screen({
        Widget{
            .sizing = { .x = { .lower = 80 }, .y = { .lower } }
        }
    });
    // Then
    REQUIRE(ctx.screen_area_is(Rect<int>{0, 0, 640, 480}));
}
