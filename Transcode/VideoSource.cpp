//
//  VideoSource.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/12/21.
//

#include "VideoSource.hpp"
#include <sstream>
#include <filesystem>
#include <fstream>

enum ConvertEvent {
    ConvertEventStart = 100,
    ConvertEventReadFrame = 101,
    ConvertEventEndFile = 102,
};
const std::string outputFolderPath = "frames";
namespace fs = std::filesystem;
VideoSource::VideoSource(VideoSourceCallback* callback)
: LXMessageHandler("videoSource") {
    this->callback = callback;
    videoReader = new VideoReader();
}

void VideoSource::exec(LXMessage* message) {
    if (message->id == ConvertEventStart) {
        videoReader->Open(m_inputPath);
        int64_t currentTime = 0;
        if (s_endTime - s_startTime < 1000000) {
            currentTime = s_startTime;
        } else {
            currentTime = s_endTime - 1000000;
        }
        this->videoReader->SetRange(currentTime, s_endTime);
        this->videoReader->Seek(currentTime);
        isFirstSeek = true;
        isFinish = false;
        LXMessage *message = new LXMessage(ConvertEventReadFrame);
        this->PostMessage(message);
    }
    if (message->id == ConvertEventReadFrame) {
        AVFrame *frame = this->videoReader->ReadVideoFrame();
        if (frame && (frameWidth == 0 || frameHeight == 0)) {
            frameWidth = frame->width;
            frameHeight = frame->height;
        }
        if (frame != nullptr) {
            std::ostringstream filePath;
            int64_t time = av_rescale_q(frame->pts, videoReader->GetTimeBase(), AV_TIME_BASE_Q);
            int64_t duration = av_rescale_q(frame->duration, videoReader->GetTimeBase(), AV_TIME_BASE_Q);
            filePath << outputFolderPath << "/frame_" << time << ".yuv";
            WriterToFile(filePath.str(), frame);
            frames.push_back({filePath.str(), time, duration});
            av_frame_free(&frame);
            frame = NULL;
            if (isFirstSeek) {
                s_startTime = time;
                isFirstSeek = false;
            }
            LXMessage *message = new LXMessage(ConvertEventReadFrame);
            this->PostMessage(message);
        } else {
            if (m_startTime - s_startTime>= 0 && isFinish) {
                callback->VideoFramesFinish();
            } else {
                callback->VideoFramesArrived(frames);
                frames.clear();
                s_endTime = s_startTime;
                s_startTime = s_startTime - 1000000;
                if (s_startTime - m_startTime < 0) {
                    s_startTime = m_startTime;
                }
                if (s_startTime == m_startTime) {
                    isFinish = true;
                }
                this->videoReader->SetRange(s_startTime, s_endTime);
                this->videoReader->Seek(s_startTime);
                isFirstSeek = true;
                LXMessage *message = new LXMessage(ConvertEventReadFrame);
                this->PostMessage(message);
            }
        }
    }
}

void VideoSource::WriterToFile(const std::string& filePath, const AVFrame* frame) {
    // 将 AVFrame 的 YUV 数据写入文件
    if (!frame || frame->format != AV_PIX_FMT_YUV420P) {
        std::cerr << "Invalid frame or unsupported format" << std::endl;
        return;
    }
    fs::path dir_path(outputFolderPath); // 指定目录路径
    // 检查目录是否存在
    if (!fs::exists(dir_path)) {
        // 创建目录
        if (fs::create_directory(dir_path)) {
            std::cout << "Directory created successfully." << std::endl;
        } else {
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    // 写入 Y 平面
    for (int y = 0; y < frame->height; y++) {
        file.write((const char*)(frame->data[0] + y * frame->linesize[0]), frame->width);
    }

    // 写入 U 平面
    for (int y = 0; y < frame->height / 2; y++) {
        file.write((const char*)(frame->data[1] + y * frame->linesize[1]), frame->width / 2);
    }

    // 写入 V 平面
    for (int y = 0; y < frame->height / 2; y++) {
        file.write((const char*)(frame->data[2] + y * frame->linesize[2]), frame->width / 2);
    }

    file.close();
    std::cout << "AVFrame written to " << filePath << std::endl;
}

void VideoSource::SetFilePath(const char *filePath) {
    m_inputPath = filePath;
}

void VideoSource::SetStartTime(int64_t startTime) {
    m_startTime = startTime;
    s_startTime = startTime;
}

void VideoSource::SetEndTime(int64_t endTime) {
    m_endTime = endTime;
    s_endTime = endTime;
}

void VideoSource::Start() {
    LXMessage *message = new LXMessage(ConvertEventStart);
    this->PostMessage(message);
}

int64_t VideoSource::GetBitrate() {
    return videoReader->GetBitrate();
}

VideoSource::~VideoSource() {
    this->callback = nullptr;
}
