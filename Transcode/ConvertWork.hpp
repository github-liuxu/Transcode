//
//  ConvertWork.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/17.
//

#ifndef ConvertWork_hpp
#define ConvertWork_hpp

#include <stdio.h>
#include <queue>
#include "LXMessageHandler.hpp"
#include "VideoReader.hpp"
#include "VideoWriter.hpp"
extern "C" {
#include <libavutil/frame.h>
}

class ConvertWork : public LXMessageHandler {
    
    class FrameInfo {
        
    };
    
public:
    ConvertWork();
    void Convert(const char* filePath, const char *outputPath, int64_t startTime, int64_t endTime);
    void Start();
    void exec(LXMessage* message);
    void NotifyVideoFrame(AVFrame *frame);
    void NotifyVideoFrameRangeEnd();
    void NotifyVideoFrameEnd();
    void NotifyWriterFinish();
private:
    VideoReader *videoReader = nullptr;
    VideoWriter *videoWriter = nullptr;
    int64_t startTime = 0;
    int64_t endTime = 0;
    const char *inputPath = NULL;
    const char *outputPath = NULL;
    std::queue<FrameInfo *> queue;
    std::queue<FrameInfo *> writerqueue;
};

#endif /* ConvertWork_hpp */
