//
//  VideoReader.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/16.
//

#ifndef VideoReader_hpp
#define VideoReader_hpp

#include <stdio.h>
#include "LXMessageHandler.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/codec.h>
    #include <libavutil/frame.h>
}

class ConvertWork;
class VideoReader : public LXMessageHandler {
    
public:
    VideoReader(ConvertWork *work);
    ~VideoReader();
    void OpenFile(const char * filePath);
    void Seek(int64_t time);
    void ReaderNextVideoFrame();
    AVFrame * GetNextVideoFrame();
    void exec(LXMessage* message);
    const char *filePath = nullptr;
private:
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *codecContext = NULL;
    ConvertWork *work = nullptr;
    std::queue<AVFrame *> frameQueue;
    std::mutex mutex;
    std::condition_variable condition;
};

#endif /* VideoReader_hpp */
