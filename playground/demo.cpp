#include <ecs.hpp>
#include <gui/core/messages.hpp>

#include <string>
#include <thread>
#include <iostream>

class Notifier {
public:
    Notifier(std::condition_variable& cv) : cv(cv) {}

    void wait(std::unique_lock<std::mutex>& lock) {
        cv.wait(lock);
    }

    void operator()() {
        cv.notify_one();
    }

private:
    std::condition_variable& cv;
};


void threadProc(
        Istok::GUI::SyncWaitingQueue<std::string>& inQueue,
        Istok::GUI::SyncNotifyingQueue<std::string, Notifier>& outQueue) {
    while (true) {
        std::string msg = inQueue.take();
        std::cout << "ecs: " << msg << std::endl;
        if (msg == "exit") {
            break;
        }
        outQueue.push("pong");
    }
    std::cout << "ecs end" << std::endl;
}


int main() {
    std::condition_variable cv;
    Notifier notifier(cv);
    Istok::GUI::SyncWaitingQueue<std::string> ecsQueue;
    Istok::GUI::SyncNotifyingQueue<std::string, Notifier> guiQueue(notifier);

    std::mutex mut;
    std::thread ecsThread(threadProc, std::ref(ecsQueue), std::ref(guiQueue));
    for (int i = 0; i < 10; ++i) {
        ecsQueue.push("ping");
        std::unique_lock lock(mut);
        notifier.wait(lock);
        std::cout << "gui: " << guiQueue.take() << std::endl;
    }
    ecsQueue.push("exit");
    ecsThread.join();
    std::cout << "gui end" << std::endl;
    return 0;
}
