//
//  FileWriter.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/12/21.
//

#ifndef FileWriter_hpp
#define FileWriter_hpp

#include <stdio.h>
#include <vector>
#include "VideoWriter.hpp"
#include "LXMessageHandler.hpp"
#include "VideoSource.hpp"
#include "VideoWriter.hpp"
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

struct FileWriterCallback {
    virtual void WriterFinish(const char *filePath) = 0;
};

class FileWriter: public LXMessageHandler {
    
public:
    FileWriter(FileWriterCallback *callback);
    void OpenFile(const char *filePath);
    bool StartFileWriter(int width, int height);
    ~FileWriter();
    void exec(LXMessage* message);
    void PushFrames(std::vector<FrameInfo> frames);
    void Finish();
private:
    AVFrame* readAVFrameFromFile(const std::string& filePath, int width, int height);
    FileWriterCallback *callback = nullptr;
    std::vector<FrameInfo> frames;
    VideoWriter *videoWriter = nullptr;
    const char *m_filePath = NULL;
    int m_width = 0;
    int m_height = 0;
};

#endif /* FileWriter_hpp */
