#include <ecs.hpp>
#include <gui/core/messages.hpp>

#include <iostream>
#include <variant>
#include <thread>


namespace CoreMsg {

struct Exit {};
struct NewWindow {};

} // namespace CoreMsg

using CoreMessage = std::variant<
    CoreMsg::Exit,
    CoreMsg::NewWindow
>;

namespace UIMsg {

struct AttachWindow {
    Istok::ECS::Entity entity;
};

} // namespace Istok::GUI::UIMsg

using UIMessage = std::variant<
    UIMsg::AttachWindow
>;


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
        Istok::GUI::SyncWaitingQueue<CoreMessage>& inQueue,
        Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier>& outQueue) {
    Istok::ECS::EntityComponentManager manager;
    while (true) {
        CoreMessage msg = inQueue.take();
        if (std::holds_alternative<CoreMsg::Exit>(msg)) {
            std::cout << "ecs <- Exit" << std::endl;
            break;
        }
        if (std::holds_alternative<CoreMsg::NewWindow>(msg)) {
            std::cout << "ecs <- NewWindow" << std::endl;
            outQueue.push(UIMsg::AttachWindow(manager.createEntity()));
        }
    }
    std::cout << "ecs end" << std::endl;
}


int main() {
    std::condition_variable cv;
    Notifier notifier(cv);
    Istok::GUI::SyncWaitingQueue<CoreMessage> ecsQueue;
    Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier> guiQueue(notifier);

    std::mutex mut;
    std::thread ecsThread(threadProc, std::ref(ecsQueue), std::ref(guiQueue));
    ecsQueue.push(CoreMsg::NewWindow{});
    {
        std::unique_lock lock(mut);
        notifier.wait(lock);
        UIMessage msg = guiQueue.take();
        if (std::holds_alternative<UIMsg::AttachWindow>(msg)) {
            std::cout << "gui <- AttachWindow("
                << std::get<UIMsg::AttachWindow>(msg).entity.value << ")"
                << std::endl;
        }
    }
    ecsQueue.push(CoreMsg::Exit{});
    ecsThread.join();
    std::cout << "gui end" << std::endl;
    return 0;
}
