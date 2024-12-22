//
//  CVideoReader.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/28.
//

#include "CVideoReader.hpp"

CVideoReader::CVideoReader()
    : formatContext(nullptr), codecContext(nullptr), videoStreamIndex(-1) {
    avformat_network_init();  // 初始化网络组件（必要时）
}

CVideoReader::~CVideoReader() {
    close();
    avformat_network_deinit();  // 清理网络组件
}

bool CVideoReader::open(const std::string& filename) {
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

    return initCodec();
}

bool CVideoReader::initCodec() {
    AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;
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

AVFrame* CVideoReader::readFrame() {
    AVFrame *frame = NULL;
    while (true) {
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatContext, packet);
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
    return frame;
    
//    while (av_read_frame(formatContext, &packet) >= 0) {
//        if (packet.stream_index == videoStreamIndex) {
//            if (avcodec_send_packet(codecContext, &packet) == 0) {
//                AVFrame* frame = av_frame_alloc();
//                if (avcodec_receive_frame(codecContext, frame) == 0) {
//                    av_packet_unref(&packet);
//                    return frame;  // 返回解码后的帧
//                }
//                av_frame_free(&frame);
//            }
//        }
//        av_packet_unref(&packet);
//    }
//    return nullptr;  // 到达文件末尾
}

int CVideoReader::getWidth() const {
    return codecContext ? codecContext->width : 0;
}

int CVideoReader::getHeight() const {
    return codecContext ? codecContext->height : 0;
}

AVRational CVideoReader::getTimeBase() const {
    return formatContext->streams[videoStreamIndex]->time_base;
}

bool CVideoReader::seek(int64_t timestamp) {
    if (av_seek_frame(formatContext, videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) >= 0) {
        avcodec_flush_buffers(codecContext);  // 清空解码器缓冲区
        return true;
    }
    return false;
}

void CVideoReader::close() {
    if (codecContext) {
        avcodec_free_context(&codecContext);
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
    av_packet_unref(&packet);
}
