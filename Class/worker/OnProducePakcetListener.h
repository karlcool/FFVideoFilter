//
// Created by ZMY on 2019/2/26.
//

#ifndef APP_ONPRODUCEPAKCETLISTENER_H
#define APP_ONPRODUCEPAKCETLISTENER_H


extern "C" {
#include <FFmpeg/avcodec.h>
};


class OnProducePakcetListener {
public:
    /**
     *获取到一帧压缩数据的回调
     * @param packet
     * @param index
     * @return  是否自己处理并回收资源 返回true的话就需要自己回收AVPacket
     */
    virtual bool onProducePacket(AVPacket *packet) = 0;

/**
 * 开始获取压缩数据的回调
 */
    virtual void onStart() = 0;

/**
 * 获取压缩数据结束（文件有问题或者读到文件尾）的回调
 */
    virtual void onEnd() = 0;
};


#endif //APP_ONPRODUCEPAKCETLISTENER_H
