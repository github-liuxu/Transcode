//
//  VideoWriter.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/24.
//

#include "VideoWriter.hpp"
#include "ConvertWork.hpp"

VideoWriter::VideoWriter(ConvertWork *work) : LXMessageHandler("VideoWriter") {
    this->work = work;
}

void VideoWriter::StartFileWriter(const char *filePath) {
    
}

void VideoWriter::exec(LXMessage* message) {
    if (message->id == 100) {
        
    }
    if (message->id == 101) {
        // 写结束
        this->work->NotifyWriterFinish();
    }
}

VideoWriter::~VideoWriter() {
    
}
