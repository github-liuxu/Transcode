//
//  LXMessageHandler.cpp
//  Composition
//
//  Created by 刘东旭 on 2024/7/18.
//

#include "LXMessageHandler.hpp"

LXMessageHandler::LXMessageHandler(std::string name) {
    this->name = name;
    worker_thread = std::thread(&LXMessageHandler::worker_function, this);
}

LXMessageHandler::~LXMessageHandler() {
    queue_condition.notify_one();
    worker_thread.join();
}

void LXMessageHandler::PostMessage(LXMessage* message) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        message_queue.push(message);
    }
    queue_condition.notify_one();
}

void LXMessageHandler::SyncMessage(LXMessage* message) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        message_queue.push(message);
    }
    queue_condition.notify_one();
    Wait();
}

void LXMessageHandler::Wait() {
    std::unique_lock<std::mutex> lock(wait_mutex);
    isWait = true;
    wait_condition.wait(lock);
}

LXMessage* LXMessageHandler::GetNextMessage() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if (message_queue.size() > 0) {
        LXMessage *message = message_queue.front();
        return message;
    } else {
        return nullptr;
    }
}

void LXMessageHandler::worker_function() {
    pthread_setname_np(name.data());
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_condition.wait(lock, [this]{ 
            return !message_queue.empty();
        });

        LXMessage* message = message_queue.front();
        message_queue.pop();
        bool empty = message_queue.empty();
        lock.unlock();
        // 处理消息的逻辑
        exec(message);
        delete message;
        if (empty && isWait) {
            wait_condition.notify_one();
        }
    }
}
