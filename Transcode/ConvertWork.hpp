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
#include "VideoSource.hpp"
#include "FileWriter.hpp"

class ConvertWork : public VideoSourceCallback,
                    public FileWriterCallback {
    
public:
    ConvertWork();
    ~ConvertWork();
    void Convert(const char* filePath, const char *outputPath, int64_t startTime, int64_t endTime);
    void Start();
    void VideoFramesArrived(std::vector<FrameInfo> frames);
    void VideoFramesFinish();
    void WriterFinish(const char *filePath);
private:
    void WriterToFile(const std::string& filePath, const AVFrame* frame);
    
    const char *inputPath = NULL;
    const char *outputPath = NULL;
    int64_t m_endTime;
    VideoSource *videoSource = nullptr;
    FileWriter *fileWriter = nullptr;
};

#endif /* ConvertWork_hpp */
