// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once


namespace Istok::Tools {

template <typename InQueue, typename OutQueue>
class Channel {
public:
    using InType = decltype(std::declval<InQueue>().take());
    using OutType = decltype(std::declval<OutQueue>().take());

    Channel() = default;
    
    Channel(
        std::shared_ptr<InQueue> inQueue,
        std::shared_ptr<OutQueue> outQueue
    ) : inQueue(inQueue), outQueue(outQueue)
    {
        assert(this->inQueue);
        assert(this->outQueue);
    }

    bool empty() {
        return inQueue->empty();
    }

    void push(OutType&& value) {
        outQueue->push(std::move(value));
    }

    void push(const OutType& value) {
        outQueue->push(value);
    }

    InType take() {
        return inQueue->take();
    }

private:
    std::shared_ptr<InQueue> inQueue;
    std::shared_ptr<OutQueue> outQueue;
};

} // namespace Istok::Tools
