//
// Created by ZMY on 2019/2/25.
//


#include <sys/time.h>
#include "Filter.h"

/**
 *
 * @param avCodecContext  需要加滤镜的流所对应的解码器AVCodecContext
 * @param filters_descr  滤镜描述 具体的滤镜效果就来自于这里
 */
Filter::Filter(AVCodecContext *avCodecContext, char *filters_descr) : filters_descr(filters_descr) {
    this->width = avCodecContext->width;
    this->height = avCodecContext->height;
    this->time_base = avCodecContext->time_base;
    this->sample_aspect_ratio = avCodecContext->sample_aspect_ratio;
    this->format = avCodecContext->pix_fmt;
    init();
}

Filter::Filter(int width, int height, AVPixelFormat format, const AVRational &time_base,
               const AVRational &sample_aspect_ratio, char *filters_descr) : width(width),
height(height),
format(format),
time_base(time_base),
sample_aspect_ratio(
                    sample_aspect_ratio),
filters_descr(
filters_descr) {
    init();
}

void Filter::init() {
    int re = 0;
    isPrepared = (re = doInit()) == 0;
    if (!isPrepared) {
        loge("init filter fail:%s", av_err2str(re));
    }
}

/**
 * 初始化滤镜
 * @return    0代表成功
 */
int Filter::doInit() {
    char args[512];
    int ret;
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    outputs = avfilter_inout_alloc();
    inputs = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = {format, AV_PIX_FMT_NONE};
    AVBufferSinkParams *buffersink_params;
    
    filter_graph = avfilter_graph_alloc();
    
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             width, height, format,
             time_base.num, time_base.den,
             sample_aspect_ratio.num, sample_aspect_ratio.den);
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        loge("Cannot create buffer source\n");
        return ret;
    }
    
    /* buffer video sink: to terminate the filter chain. */
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        logd("Cannot create buffer sink,ret=%d\n", ret);
        return ret;
    }
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;
    
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;
    
    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        return ret;
    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        return ret;
    return 0;
}

void Filter::doWork(AVFrame *input) {
    if (isPrepared) {
        if (av_buffersrc_add_frame(buffersrc_ctx, input) < 0) {
            loge("Could not av_buffersrc_add_frame");
        }
        if (av_buffersink_get_frame(buffersink_ctx, input) < 0) {
            loge("Could not av_buffersink_get_frame");
        }
    }
}

int Filter::getIsPrepared() const {
    return isPrepared;
}

Filter::~Filter() {
    release();
}

void Filter::release() {
    if (inputs != nullptr) {
        avfilter_inout_free(&inputs);
        inputs = nullptr;
    }
    if (outputs != nullptr) {
        avfilter_inout_free(&outputs);
        inputs = nullptr;
    }
    if (filter_graph != nullptr) {
        avfilter_graph_free(&filter_graph);
        filter_graph = nullptr;
    }
}

char *Filter::getFilters_descr() const {
    return filters_descr;
}
