//
//  VideoReader.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/28.
//

#ifndef CVideoReader_hpp
#define CVideoReader_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include <memory>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/avutil.h>
    #include <libavutil/pixdesc.h>
}

class VideoReader {
public:
    VideoReader();
    ~VideoReader();
    bool open(const std::string& filename);
    void close();
    AVFrame* readVideoFrame();
    int getWidth() const;
    int getHeight() const;
    AVRational getTimeBase() const;
    bool seek(int64_t timestamp);
private:
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    int videoStreamIndex;
    AVPacket packet;
    bool initCodec();
    void cleanup();
};


#endif /* CVideoReader_hpp */
