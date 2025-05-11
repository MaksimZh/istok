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
void dialog_created() {}
void user_clicks_button(string id) {}
bool dialog_closed() { return true; }
bool dialog_returned(string value) { return true; }
};


TEST_CASE("Test OK", "[gui]") {
    Context ctx;
    ctx.dialog_created();
    
    ctx.user_clicks_button("OK");
    
    REQUIRE(ctx.dialog_closed());
    REQUIRE(ctx.dialog_returned("OK"));
}


TEST_CASE("Test Cancel", "[gui]") {
    Context ctx;
    ctx.dialog_created();
    
    ctx.user_clicks_button("Cancel");
    
    REQUIRE(ctx.dialog_closed());
    REQUIRE(ctx.dialog_returned("Cancel"));
}
