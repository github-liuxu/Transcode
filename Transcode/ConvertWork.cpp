//
//  ConvertWork.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/17.
//

#include "ConvertWork.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

enum ConvertEvent {
    ConvertEventStartWriter,
    ConvertEventWriterFrame,
    ConvertEventWriterEnd,
};

ConvertWork::ConvertWork() {
    videoSource = new VideoSource(this);
}

ConvertWork::~ConvertWork() {
    delete videoSource;
    videoSource = nullptr;
    delete fileWriter;
    fileWriter = nullptr;
}
void ConvertWork::Convert(const char* filePath, const char *outputPath, int64_t startTime, int64_t endTime) {
    videoSource->SetFilePath(filePath);
    videoSource->SetStartTime(startTime);
    videoSource->SetEndTime(endTime);
    m_endTime = endTime;
    this->outputPath = outputPath;
}

void ConvertWork::Start() {
    videoSource->Start();
}

void ConvertWork::VideoFramesArrived(std::vector<FrameInfo> frames) {
    if (fileWriter == nullptr) {
        fileWriter = new FileWriter(this);
        fileWriter->OpenFile(outputPath);
        fileWriter->StartFileWriter(videoSource->frameWidth, videoSource->frameHeight);
    }
    std::vector<FrameInfo> w_frames;
    size_t size = frames.size();
    for (int i = 0; i < size; i++) {
        FrameInfo info = frames[size - 1 - i];
        info.pts = m_endTime - info.pts - info.duration;
        w_frames.push_back(info);
    }
    fileWriter->PushFrames(w_frames);
}

void ConvertWork::VideoFramesFinish() {
    fileWriter->Finish();
}

void ConvertWork::WriterFinish(const char *filePath) {
    std::cout << "filePath: " << filePath;
}
