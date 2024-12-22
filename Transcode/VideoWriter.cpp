//
//  VideoWriter.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/24.
//

#include "VideoWriter.hpp"
#include "ConvertWork.hpp"

VideoWriter::VideoWriter() {
}

void VideoWriter::OpenFile(const char *filePath) {
    this->filePath = filePath;
}

bool VideoWriter::StartFileWriter(int width, int height, AVRational time_base) {
    avformat_alloc_output_context2(&formatCtx, nullptr, nullptr, filePath);
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    codecCtx = avcodec_alloc_context3(codec);
    codecCtx->width = width;
    codecCtx->height = height;
    AVRational rat;
    rat.num = 1;
    rat.den = 30;
    codecCtx->time_base = rat;
    codecCtx->gop_size = 15;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    AVDictionary *opts = NULL;
    avcodec_open2(codecCtx, codec, &opts);
    AVStream* stream = avformat_new_stream(formatCtx, codec);
    stream->index = 0;
    stream->codecpar->codec_id = codec->id;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = width;
    stream->codecpar->height = height;
    stream->codecpar->format = codecCtx->pix_fmt;
    stream->time_base = time_base;

    avio_open(&formatCtx->pb, filePath, AVIO_FLAG_WRITE);
    return !avformat_write_header(formatCtx, nullptr);
}

bool VideoWriter::WriterVideoFrame(AVFrame *frame, int64_t pts) {
    int ret = avcodec_send_frame(codecCtx, frame);
    while (ret >= 0) {
        AVPacket *pkt = av_packet_alloc();
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN)) {
            std::cout << "encodeRet:" << ret << std::endl;
            av_packet_unref(pkt);
            av_packet_free(&pkt);
            break;
        } else if (ret < 0) {
            av_packet_unref(pkt);
            av_packet_free(&pkt);
            break;
        }
        pkt->pts = pts;
        pkt->time_base = frame->time_base;
        av_interleaved_write_frame(formatCtx, pkt);
        av_packet_unref(pkt);
        av_packet_free(&pkt);
    }
    
    return true;
}

void VideoWriter::WriterTrailer(){
    av_write_trailer(formatCtx);
    avcodec_free_context(&codecCtx);
    avio_close(formatCtx->pb);
    avformat_free_context(formatCtx);
}

VideoWriter::~VideoWriter() {
    
}
