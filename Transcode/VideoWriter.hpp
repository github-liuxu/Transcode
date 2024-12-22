//
//  VideoWriter.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/24.
//

#ifndef VideoWriter_hpp
#define VideoWriter_hpp

#include <stdio.h>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/avutil.h>
}
class ConvertWork;
class VideoWriter {
    
public:
    VideoWriter();
    ~VideoWriter();
    void OpenFile(const char *filePath);
    bool StartFileWriter(int width, int height, AVRational time_base);
    bool WriterVideoFrame(AVFrame *frame, int64_t pts);
    void WriterTrailer();
    const char *filePath = nullptr;
    ConvertWork *work = nullptr;
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
};

#endif /* VideoWriter_hpp */
