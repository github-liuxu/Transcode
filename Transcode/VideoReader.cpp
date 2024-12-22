//
//  VideoReader.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/16.
//

#include "VideoReader.hpp"
#include "ConvertWork.hpp"
#include "CVideoReader.hpp"

CVideoReader *reader = nullptr;
VideoReader::VideoReader(ConvertWork *work)
    :LXMessageHandler("VideoReader") {
    this->work = work;
        reader = new CVideoReader();
}

void VideoReader::OpenFile(const char *filePath) {
    reader->open(std::string(filePath));
}

void VideoReader::Seek(int64_t time) {
    reader->seek(time);
}

void VideoReader::ReaderNextVideoFrame() {
    AVFrame *frame = NULL;
    while (true) {
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(fmt_ctx, packet);
        if (ret < 0) {
            std::cout << "read frame error" << std::endl;
            av_packet_unref(packet);
            av_packet_free(&packet);
            return;
        }
        ret = avcodec_send_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN)) {
            av_packet_unref(packet);
            av_packet_free(&packet);
            continue;
        }
        if (ret == AVERROR_EOF) {
            av_packet_unref(packet);
            av_packet_free(&packet);
            return;
        }
        if (ret != 0) {
            std::cout << "avcodec_send_packet error" << std::endl;
            av_packet_unref(packet);
            av_packet_free(&packet);
            return;
        }
        frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == 0) {
            break;
        }
        if (ret == AVERROR(EAGAIN)) {
            av_frame_unref(frame);
            av_frame_free(&frame);
            continue;
        }
        if (ret == AVERROR_EOF) {
            av_frame_unref(frame);
            av_frame_free(&frame);
            return;
        }
        if (ret != 0) {
            std::cout << "avcodec_send_packet error" << std::endl;
            av_frame_unref(frame);
            av_frame_free(&frame);
            return;
        }
    }
    if (frame != NULL) {
        std::unique_lock<std::mutex> lock(mutex);
        frameQueue.push(frame);
        condition.notify_one();
    }
    LXMessage *message = new LXMessage(100);
    this->PostMessage(message);
//    this->work->NotifyVideoFrame(nullptr);
}

AVFrame *VideoReader::GetNextVideoFrame() {
    std::unique_lock<std::mutex> lock(mutex);
    if (frameQueue.size() == 0) {
        condition.wait(lock);
    }
    AVFrame *frame = frameQueue.front();
    frameQueue.pop();
    return frame;
}

void VideoReader::exec(LXMessage* message) {
    if (message->id == 100) {
        ReaderNextVideoFrame();
    }
}

VideoReader::~VideoReader() {
    
}
