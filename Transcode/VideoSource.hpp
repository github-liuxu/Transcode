//
//  VideoSource.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/12/21.
//

#ifndef VideoSource_hpp
#define VideoSource_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include "LXMessageHandler.hpp"
#include "VideoReader.hpp"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

struct FrameInfo {
    std::string filePath;
    int64_t pts;
    int64_t duration;
};

struct VideoSourceCallback {
    virtual void VideoFramesArrived(std::vector<FrameInfo> frames) = 0;
    virtual void VideoFramesFinish() = 0;
};

class VideoSource : public LXMessageHandler {
public:
    int frameWidth = 0;
    int frameHeight = 0;
public:
    VideoSource(VideoSourceCallback* callback);
    ~VideoSource();
    void exec(LXMessage* message);
    void SetFilePath(const char *filePath);
    void SetStartTime(int64_t startTime);
    void SetEndTime(int64_t endTime);
    void Start();
    void WriterToFile(const std::string& filePath, const AVFrame* frame);
    int64_t GetBitrate();
private:
    VideoSourceCallback* callback;
    std::vector<FrameInfo> frames;
    int64_t m_startTime = 0;
    int64_t m_endTime = 0;
    int64_t s_startTime = 0;
    int64_t s_endTime = 0;
    bool isFirstSeek = false;
    bool isFinish = false;
    const char *m_inputPath = NULL;
    VideoReader *videoReader = nullptr;
};

#endif /* VideoSource_hpp */
