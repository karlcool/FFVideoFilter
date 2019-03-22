//
// Created by ZMY on 2019/2/25.
//

#include "Decoder.h"
#include "common_utils.h"

Decoder::Decoder(AVCodecContext *codecContext) : codecContext(codecContext) {
}

OnDecodeFrameListener *Decoder::getListener() const {
    return listener;
}

void Decoder::setListener(OnDecodeFrameListener *listener) {
    Decoder::listener = listener;
}

AVCodecContext *Decoder::getCodecContext() const {
    return codecContext;
}

void Decoder::setCodecContext(AVCodecContext *codecContext) {
    Decoder::codecContext = codecContext;
}

Decoder::Decoder() {}

/**
 * 编码
 * @param packet
 * @param index
 * @param type
 */
void Decoder::decode(AVPacket *packet, int index, int type) {
    int got = 0;
    int result = 0;
    AVFrame *frame = av_frame_alloc();
    if (type == 0) {
        result = avcodec_decode_video2(codecContext, frame, &got, packet);
        if (result < 0) {
            loge("视频 解码错误----res=%s", av_err2str(result));
            av_frame_free(&frame);
        }
    } else if (type == 1) {
        result = avcodec_decode_audio4(codecContext, frame, &got, packet);
        if (result < 0) {
            loge("视频 解码错误----res=%s", av_err2str(result));
            av_frame_free(&frame);
        }
    }
    if (result < 0) {
        loge("视频 解码错误----res=%s", av_err2str(result));
        av_frame_free(&frame);
    }
    if (got) {
        if (listener == nullptr || !listener->onDecodeFrame(frame, index, type)) {
            av_frame_free(&frame);
        }
    } else {
        av_frame_free(&frame);
    }
}
