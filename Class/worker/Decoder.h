//
// Created by ZMY on 2019/2/25.
//

#ifndef APP_DECODER_H
#define APP_DECODER_H


extern "C" {
#include <FFmpeg/avcodec.h>
};

#include "OnDecodeFrameListener.h"

class Decoder {
private:
    AVCodecContext *codecContext = nullptr;
    OnDecodeFrameListener *listener = nullptr;  //回调

public:

    Decoder();

    Decoder(AVCodecContext *codecContext);

    OnDecodeFrameListener *getListener() const;

    void setListener(OnDecodeFrameListener *listener);

    AVCodecContext *getCodecContext() const;

    void setCodecContext(AVCodecContext *codecContext);

    void decode(AVPacket *packet, int index, int type);//0视频  1音频
};


#endif //APP_DECODER_H
