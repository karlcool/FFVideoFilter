//
// Created by ZMY on 2019/2/25.
//

#ifndef APP_FILTER_H
#define APP_FILTER_H


extern "C" {
#include <FFmpeg/avcodec.h>
#include <FFmpeg/avfilter.h>
#include <FFmpeg/buffersink.h>
#include <FFmpeg/buffersrc.h>
    
};

#include "common_utils.h"

class Filter {
private:
    int width = 0;
    int height = 0;
    AVPixelFormat format;
    AVRational time_base;
    AVRational sample_aspect_ratio;
    
    
    AVFilterGraph *filter_graph = nullptr;
    AVFilterInOut *outputs = nullptr;
    AVFilterInOut *inputs = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;
    AVFilterContext *buffersrc_ctx = nullptr;
    char *filters_descr = nullptr;
    int isPrepared = false;
    
    void init();
    
    void release();
    
public:
    Filter(AVCodecContext *ctx, char *filters_descr);
    
    Filter(int width, int height, AVPixelFormat format, const AVRational &time_base,
           const AVRational &sample_aspect_ratio, char *filters_descr);
    
    int doInit();
    
    void doWork(AVFrame *input);
    
    int getIsPrepared() const;
    
    virtual ~Filter();
    
    char *getFilters_descr() const;
    
};


#endif //APP_FILTER_H
