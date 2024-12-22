//
//  LXMessageHandler.hpp
//  Composition
//
//  Created by 刘东旭 on 2024/7/18.
//

#ifndef LXMessageHandler_hpp
#define LXMessageHandler_hpp

#include <stdio.h>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class LXMessage {
public:
    int id;
public:
    LXMessage(){};
    LXMessage(int id){ this->id = id; };
    ~LXMessage(){};
};

class LXMessageHandler {
public:
    LXMessageHandler(std::string name);
    ~LXMessageHandler();
    void PostMessage(LXMessage* message);
    void SyncMessage(LXMessage* message);
    void worker_function();
    void Wait();
    LXMessage* GetNextMessage();
protected:
    virtual void exec(LXMessage* message) = 0;
private:
    std::thread worker_thread;
    std::queue<LXMessage*> message_queue;
    std::mutex queue_mutex;
    std::mutex wait_mutex;
    std::condition_variable queue_condition;
    std::condition_variable wait_condition;
    std::string name;
    bool isWait = false;
};


#endif /* LXMessageHandler_hpp */
