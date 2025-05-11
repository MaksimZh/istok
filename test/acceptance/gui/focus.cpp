/*
    Copyright 2025 Maksim Sergeevich Zholudev

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <catch.hpp>

#include <iostream>
using namespace std;


class Context {
public:

    void window_created() {
        //TODO
        //Create window with 2 labels 2 buttons and 2 text fields
    }

    void mouse_over_empty_space() {
        //TODO
        //Simulate mouse move over space between widgets
    }

    int idle_sprites_count() {
        //TODO
        //Count idle sprites
        return 4;
    }
};


TEST_CASE("Test Hover", "[gui]") {
    Context ctx;
    ctx.window_created();
    ctx.mouse_over_empty_space();
    REQUIRE(ctx.idle_sprites_count() == 4);
}
