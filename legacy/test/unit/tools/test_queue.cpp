// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/queue.hpp>

using namespace Istok::Tools;

#include <memory>
#include <thread>
#include <chrono>
#include <semaphore>

using namespace std::chrono_literals;

TEST_CASE("Tools - simple queue", "[unit][tools]") {
    SimpleQueue<int> queue;
    REQUIRE(queue.empty() == true);

    SECTION("push and take") {
        queue.push(42);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 42);
        REQUIRE(queue.empty() == true);
    }

    SECTION("multiple items") {
        queue.push(0);
        int a = 1;
        queue.push(a);
        queue.push(2);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 0);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 2);
        REQUIRE(queue.empty() == true);
    }

    SECTION("mixed scenario") {
        queue.push(0);
        queue.push(1);
        REQUIRE(queue.take() == 0);
        queue.push(2);
        queue.push(3);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        REQUIRE(queue.empty() == false);
        queue.push(4);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
    }
}


TEST_CASE("Tools - simple queue - moving", "[unit][tools]") {
    SimpleQueue<std::unique_ptr<int>> queue;
    REQUIRE(queue.empty() == true);

    auto a = std::make_unique<int>(42);
    queue.push(std::move(a));
    REQUIRE(queue.empty() == false);
    auto b = queue.take();
    REQUIRE(b != nullptr);
    REQUIRE(*b == 42);
    REQUIRE(queue.empty() == true);
}


TEST_CASE("Tools - waiting queue", "[unit][tools]") {
    WaitingQueue<int> queue;
    REQUIRE(queue.empty() == true);

    SECTION("linear usage") {
        std::mutex mut;
        std::unique_lock lock(mut);
        queue.push(0);
        int a = 1;
        queue.push(a);
        REQUIRE(queue.take(lock) == 0);
        queue.push(2);
        queue.push(3);
        REQUIRE(queue.take(lock) == 1);
        REQUIRE(queue.take(lock) == 2);
        queue.push(4);
        REQUIRE(queue.take(lock) == 3);
        REQUIRE(queue.take(lock) == 4);
        REQUIRE(queue.empty() == true);
    }

    SECTION("waiting") {
        std::mutex mut;
        std::thread second([&]{
            for (int i = 0; i < 20; ++i) {
                std::this_thread::sleep_for(1ms);
                std::lock_guard lock(mut);
                queue.push(std::move(i));
            }
        });
        for (int i = 0; i < 20; ++i) {
            std::unique_lock lock(mut);
            REQUIRE(queue.take(lock) == i);
        }
        second.join();
    }
}


TEST_CASE("Tools - synchronized waiting queue", "[unit][tools]") {
    SyncWaitingQueue<int> queue;
    REQUIRE(queue.empty() == true);

    SECTION("linear usage") {
        queue.push(0);
        int a = 1;
        queue.push(a);
        REQUIRE(queue.take() == 0);
        queue.push(2);
        queue.push(3);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        queue.push(4);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
    }

    SECTION("muti-thread usage") {
        std::thread second([&queue]{
            for (int i = 0; i < 20; ++i) {
                queue.push(std::move(i));
                std::this_thread::sleep_for(1ms);
            }
        });
        for (int i = 0; i < 20; ++i) {
            REQUIRE(queue.take() == i);
            std::this_thread::sleep_for(1ms);
        }
        second.join();
    }
}


TEST_CASE("Tools - notifying queue", "[unit][tools]") {
    size_t counter = 0;
    auto inc = [&]() noexcept {++counter;};
    NotifyingQueue<int, decltype(inc)> queue(std::move(inc));
    REQUIRE(queue.empty() == true);
    REQUIRE(counter == 0);

    SECTION("take from empty") {
        REQUIRE(queue.take().has_value() == false);
    }

    SECTION("push + take") {
        queue.push(0);
        REQUIRE(queue.empty() == false);
        REQUIRE(counter == 1);
        int a = 1;
        queue.push(a);
        REQUIRE(counter == 2);
        REQUIRE(queue.take() == 0);
        REQUIRE(counter == 2);
        queue.push(2);
        REQUIRE(counter == 3);
        queue.push(3);
        REQUIRE(counter == 4);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        queue.push(4);
        REQUIRE(counter == 5);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
        REQUIRE(counter == 5);
    }
}


TEST_CASE("Tools - synchronized notifying queue", "[unit][tools]") {
    size_t counter = 0;
    auto inc = [&]() noexcept {++counter;};
    SyncNotifyingQueue<int, decltype(inc)> queue(std::move(inc));
    REQUIRE(queue.empty() == true);
    REQUIRE(counter == 0);

    SECTION("take from empty") {
        REQUIRE(queue.take().has_value() == false);
    }

    SECTION("push + take") {
        queue.push(0);
        REQUIRE(queue.empty() == false);
        REQUIRE(counter == 1);
        int a = 1;
        queue.push(a);
        REQUIRE(counter == 2);
        REQUIRE(queue.take() == 0);
        REQUIRE(counter == 2);
        queue.push(2);
        REQUIRE(counter == 3);
        queue.push(3);
        REQUIRE(counter == 4);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        queue.push(4);
        REQUIRE(counter == 5);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
        REQUIRE(counter == 5);
    }
}


TEST_CASE("Tools - synchronized notifying queue - multithread", "[unit][tools]") {
    std::counting_semaphore sem{0};
    auto notifier = [&]() noexcept { sem.release(); };
    SyncNotifyingQueue<int, decltype(notifier)> queue(std::move(notifier));
    REQUIRE(queue.empty() == true);

    std::thread thread([&]{
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(1ms);
            queue.push(std::move(i));
        }
        for (int i = 10; i < 20; ++i) {
            std::this_thread::sleep_for(1ms);
            queue.push(i);
        }
    });
    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(1ms);
        sem.acquire();
        REQUIRE(queue.take() == i);
    }
    thread.join();
}
