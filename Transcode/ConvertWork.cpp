//
//  ConvertWork.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/17.
//

#include "ConvertWork.hpp"
#include <unistd.h>

ConvertWork::ConvertWork():LXMessageHandler("ConvertWork") {
    this->videoReader = new VideoReader(this);
    this->videoWriter = new VideoWriter(this);
}

void ConvertWork::Convert(const char* filePath, const char *outputPath, int64_t startTime, int64_t endTime) {
    this->inputPath = filePath;
    this->outputPath = outputPath;
    this->startTime = startTime;
    this->endTime = endTime;
}

void ConvertWork::Start() {
    LXMessage *message = new LXMessage(100);
    this->PostMessage(message);
}

void ConvertWork::NotifyVideoFrame(AVFrame *frame) {
    FrameInfo *info = new FrameInfo();
    queue.push(info);
}

void ConvertWork::NotifyVideoFrameRangeEnd() {
    while (true) {
        if (queue.size() > 0) {
            this->writerqueue.push(queue.back());
            queue.pop();
        } else {
            break;
        }
    }
    LXMessage *writerMessage = new LXMessage(100);
    this->videoWriter->PostMessage(writerMessage);
    
    this->endTime = this->startTime;
    if (this->startTime < 1000000 && this->startTime> 0) {
        this->startTime = 0;
    } else {
        this->startTime -= 1;
    }
    videoReader->Seek(this->startTime);
    LXMessage *message = new LXMessage(100);
    this->videoReader->PostMessage(message);
}

void ConvertWork::NotifyVideoFrameEnd() {
    // 读结束
    LXMessage *writerMessage = new LXMessage(101);
    this->videoWriter->PostMessage(writerMessage);
}

void ConvertWork::NotifyWriterFinish() {
    std::cout << "写完成" << std::endl;
}

void ConvertWork::exec(LXMessage* message) {
    if (message->id == 100) {
        this->startTime = this->endTime - 1000000;
        videoReader->OpenFile(this->inputPath);
        videoReader->Seek(this->startTime);
        LXMessage *message = new LXMessage(100);
        this->videoReader->PostMessage(message);
        this->videoWriter->StartFileWriter(this->outputPath);
    }
}
