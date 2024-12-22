//
//  VideoReader.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/28.
//

#include "VideoReader.hpp"

VideoReader::VideoReader()
    : formatContext(nullptr), codecContext(nullptr), videoStreamIndex(-1) {
    avformat_network_init();  // 初始化网络组件（必要时）
}

VideoReader::~VideoReader() {
    Close();
    avformat_network_deinit();  // 清理网络组件
}

bool VideoReader::Open(const std::string& filename) {
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Failed to find stream info" << std::endl;
        return false;
    }

    videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "No video stream found" << std::endl;
        return false;
    }

    AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;
    bitRate = codecParams->bit_rate;
    const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Failed to find decoder" << std::endl;
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "Failed to allocate codec context" << std::endl;
        return false;
    }

    if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
        std::cerr << "Failed to copy codec parameters" << std::endl;
        return false;
    }
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        return false;
    }

    return true;
}

void VideoReader::SetRange(int64_t start, int64_t end) {
    m_start = start;
    m_end = end;
}

int64_t VideoReader::GetBitrate() {
    return codecContext ? codecContext->bit_rate : 0;
}

AVFrame* VideoReader::ReadVideoFrame() {
    AVFrame *frame = NULL;
    while (true) {
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatContext, packet);
        if (packet->stream_index != videoStreamIndex) {
            continue;
        }
        if (ret < 0) {
            std::cout << "read frame error" << std::endl;
            av_packet_unref(packet);
            av_packet_free(&packet);
            return nullptr;
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
            return nullptr;
        }
        if (ret != 0) {
            std::cout << "avcodec_send_packet error" << std::endl;
            av_packet_unref(packet);
            av_packet_free(&packet);
            return nullptr;
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
            return nullptr;
        }
        if (ret != 0) {
            std::cout << "avcodec_send_packet error" << std::endl;
            av_frame_unref(frame);
            av_frame_free(&frame);
            return nullptr;
        }
    }
    int64_t time = av_rescale_q(frame->pts, GetTimeBase(), AV_TIME_BASE_Q);
    if (time >= m_end) {
        av_frame_unref(frame);
        av_frame_free(&frame);
        return nullptr;
    }
    return frame;
}

int VideoReader::GetWidth() const {
    return codecContext ? codecContext->width : 0;
}

int VideoReader::GetHeight() const {
    return codecContext ? codecContext->height : 0;
}

AVRational VideoReader::GetTimeBase() const {
    return formatContext->streams[videoStreamIndex]->time_base;
}

int64_t VideoReader::GetDuration() const {
    return formatContext->streams[videoStreamIndex]->duration;
}

bool VideoReader::Seek(int64_t timestamp) {
    int64_t time = av_rescale_q(timestamp, AV_TIME_BASE_Q, GetTimeBase());
    if (av_seek_frame(formatContext, videoStreamIndex, time, AVSEEK_FLAG_BACKWARD) >= 0) {
        avcodec_flush_buffers(codecContext);  // 清空解码器缓冲区
        return true;
    }
    return false;
}

void VideoReader::Close() {
    if (codecContext) {
        avcodec_free_context(&codecContext);
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}
