//
//  FileWriter.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/12/21.
//

#include "FileWriter.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/error.h>
    #include <libavutil/samplefmt.h>
}

class LXWriterMessage: public LXMessage {
    
public:
    LXWriterMessage(int id, std::vector<FrameInfo> frames):LXMessage(id) {
        this->frames = frames;
    }
public:
    std::vector<FrameInfo> frames;
};

FileWriter::FileWriter(FileWriterCallback *callback)
: LXMessageHandler("FileWriter") {
    videoWriter = new VideoWriter();
}

FileWriter::~FileWriter() {
    
}

void FileWriter::exec(LXMessage* message) {
    if (message->id == 100) {
        while (frames.size() > 0) {
            FrameInfo info = frames.front();
            frames.erase(frames.begin());
            AVFrame* frame = readAVFrameFromFile(info.filePath, m_width, m_height);
            remove(info.filePath.data());
            frame->time_base = AV_TIME_BASE_Q;
            videoWriter->WriterVideoFrame(frame, info.pts);
            std::cout << "info.pts: " << info.pts << std::endl;
            av_frame_free(&frame);
            frame = NULL;
        }
    } else if (message->id == 101) {
        videoWriter->WriterTrailer();
    } else if (message->id == 102) {
        LXWriterMessage *m = (LXWriterMessage *)message;
        for (FrameInfo frameInfo : m->frames) {
            this->frames.push_back(frameInfo);
        }
        LXMessage *message = new LXMessage(100);
        this->PostMessage(message);
    }
}

void FileWriter::OpenFile(const char *filePath) {
    m_filePath = filePath;
    videoWriter->OpenFile(filePath);
}

bool FileWriter::StartFileWriter(int width, int height) {
    m_width = width;
    m_height = height;
    return videoWriter->StartFileWriter(width, height, AV_TIME_BASE_Q);
}

void FileWriter::PushFrames(std::vector<FrameInfo> frames) {
    LXWriterMessage *message = new LXWriterMessage(102, frames);
    this->PostMessage(message);
}

void FileWriter::Finish() {
    LXMessage *message = new LXMessage(101);
    this->PostMessage(message);
}

// 从文件中读取 YUV 数据并填充到 AVFrame
AVFrame* FileWriter::readAVFrameFromFile(const std::string& filePath, int width, int height) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return nullptr;
    }

    // 创建并初始化 AVFrame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Failed to allocate AVFrame" << std::endl;
        return nullptr;
    }

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;

    int ret = av_image_alloc(frame->data, frame->linesize, width, height, (AVPixelFormat)frame->format, 32);
    if (ret < 0) {
        std::cerr << "Failed to allocate image buffer" << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }

    // 读取 Y 平面
    for (int y = 0; y < height; y++) {
        file.read((char*)(frame->data[0] + y * frame->linesize[0]), width);
    }

    // 读取 U 平面
    for (int y = 0; y < height / 2; y++) {
        file.read((char*)(frame->data[1] + y * frame->linesize[1]), width / 2);
    }

    // 读取 V 平面
    for (int y = 0; y < height / 2; y++) {
        file.read((char*)(frame->data[2] + y * frame->linesize[2]), width / 2);
    }

    if (file.fail()) {
        std::cerr << "Failed to read file data" << std::endl;
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
        return nullptr;
    }

    file.close();
    std::cout << "AVFrame read from " << filePath << std::endl;
    return frame;
}
