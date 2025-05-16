// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a user
    I want to get visual feedback from gui
    to be sure the application interprets my commands correctly
*/


class Context {
public:
    //TODO
    //WindowManager wm;
    //Window window;

    void window_created() {
        //TODO
        /*
        window = wm.createWindow("Test", {
            stackX({
                spaceX(16),
                stackY({
                    spaceY(16),
                    label("label1"),
                    spaceY(16),
                    textField(),
                    spaceY(16),
                    button("button1"),
                    spaceY(16),
                }),
                spaceX(16),
                stackY({
                    spaceY(16),
                    label("label2"),
                    spaceY(16),
                    textField(),
                    spaceY(16),
                    button("button2"),
                    spaceY(16),
                }),
                spaceX(16),
            })
        });
        */
    }

    void mouse_over_empty_space() {
        //TODO
        /*
        window.mouseMove();
        */
    }

    bool all_controls_idle() {
        //TODO
        return true;
    }
};


TEST_CASE("Test Hover", "[gui]") {
    Context ctx;
    ctx.window_created();
    ctx.mouse_over_empty_space();
    REQUIRE(ctx.all_controls_idle());
}
