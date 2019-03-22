//
// Created by ZMY on 2019/2/26.
//

#ifndef APP_ONDECODEFRAMELISTENER_H
#define APP_ONDECODEFRAMELISTENER_H


extern "C" {
#include <FFmpeg/frame.h>
};

class OnDecodeFrameListener {
public:
    /**
    *
    * @param frame 视频帧的原始数据（这是可以用来播放的数据）
    * @param type 0:视频  1:音频
    * @return  是否自己处理并回收AVFrame  返回true的话就需要自己回收AVFrame
    */
    virtual bool onDecodeFrame(AVFrame *frame, int index, int type) = 0;
};


#endif //APP_ONDECODEFRAMELISTENER_H
