//
// Created by ZMY on 2019/2/25.
//

#include "Encoder.h"
#include "common_utils.h"

char *Encoder::getOutputFile() const {
    return outputFile;
}


/**
 * 根据文件滤镜 生成新的AVFormatContext
 * @param out_file
 */
void Encoder::setOutputFile(char *out_file) {
    Encoder::outputFile = out_file;
    int re = 0;
    if ((re = avformat_alloc_output_context2(&outputFormat, nullptr, nullptr, out_file)) < 0) {
        loge("avformat_alloc_output_context2 fail :%s \n",
             av_err2str(re));
        isPrepared = false;
        return;
    }

    //Open output URL
    if ((re = avio_open(&outputFormat->pb, out_file, AVIO_FLAG_READ_WRITE)) < 0) {
        printf("Failed to open output file: \n", av_err2str(re));
        return;
    }

}


Encoder::Encoder(char *outputFile) {
    setOutputFile(outputFile);
}

/**
 * 为AVFormatContext添加流
 * @param codecContext  源数据的解码器
 * @param index         源数据所对应的流的index
 * @param type  0视频流，1 音频流
 * @param needOpenCodec    是否需要初始化新流的编码器
 * @return  是否成功
 */
bool Encoder::init(AVFormatContext *fmt, int index, int type, bool needOpenCodec,
                   map<char *, char *> *option) {
    int re = 0;
    AVStream *iStream = fmt->streams[index];
    AVCodecContext *codecContext = iStream->codec;
    AVCodecContext *outPutCodecCtx = nullptr;
    AVCodec *outCodec = nullptr;
    if (type == 0) {
        outCodec = avcodec_find_encoder(AV_CODEC_ID_H264);  //视频  强制使用h264编码器
    } else if (type == 1) {
        outCodec = avcodec_find_encoder(codecContext->codec_id); //音频保持原有编解码格式
    }
    if (!outCodec) {
        loge("Can not find encoder! \n");
        return false;
    }
    outPutCodecCtx = avcodec_alloc_context3(outCodec);
    outPutCodecCtx->codec_type = codecContext->codec_type;
    outPutCodecCtx->frame_size = codecContext->frame_size;
    outPutCodecCtx->frame_number = codecContext->frame_number;
    outPutCodecCtx->framerate = codecContext->framerate;
    outPutCodecCtx->pix_fmt = codecContext->pix_fmt;
    outPutCodecCtx->width = codecContext->width;
    outPutCodecCtx->height = codecContext->height;
    outPutCodecCtx->bit_rate = codecContext->bit_rate;
    outPutCodecCtx->gop_size = codecContext->gop_size;
//    outPutCodecCtx->time_base = {1, codecContext->time_base.den / codecContext->time_base.num};
    outPutCodecCtx->time_base = codecContext->time_base;
    outPutCodecCtx->max_b_frames = codecContext->max_b_frames;
    outPutCodecCtx->sample_rate = codecContext->sample_rate;
    outPutCodecCtx->sample_fmt = codecContext->sample_fmt;
//    outPutCodecCtx->pkt_timebase = {1, codecContext->pkt_timebase.den /
//                                       codecContext->pkt_timebase.num};
    outPutCodecCtx->pkt_timebase = codecContext->pkt_timebase;
    outPutCodecCtx->channels = codecContext->channels;
    outPutCodecCtx->channel_layout = codecContext->channel_layout;

    //设置该标志位后  在write_header时就会自动填充sps,pps等信息，否则普通播放器会无法播放该视频（专业播放器除外）
    if (outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
        outPutCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVStream *stream = avformat_new_stream(outputFormat, outCodec);  //创建新流
    if (stream == nullptr) {
        loge("new Stream fail");
        return false;
    }
    stream->r_frame_rate = iStream->r_frame_rate;
    stream->avg_frame_rate = iStream->avg_frame_rate;
    stream->time_base = iStream->time_base;
    if (option != nullptr) {
        auto it = option->begin();
        while (it != option->end()) {
            av_dict_set(&stream->metadata, it->first, it->second, 0); //设置旋转角度
            it++;
        }
    }
    stream->codec = outPutCodecCtx;
    if (needOpenCodec) {
        AVDictionary *param = nullptr;
        //H.264
        if (outPutCodecCtx->codec_id == AV_CODEC_ID_H264) {
            av_dict_set(&param, "preset", "ultrafast", 0);  //设置为ultrafast   将加快编码速度
            av_dict_set(&param, "tune", "zerolatency", 0);
        }
        if ((re = avcodec_open2(outPutCodecCtx, outPutCodecCtx->codec, &param)) < 0) {
            loge("Failed to open encoder: errorCode=%d ,%s \n", re, av_err2str(re));
            return false;
        }
    }
    openedCodec[index] = stream->index;
    return true;
}

/**
 * 编码  并将编码后的数据写入文件
 */
void Encoder::encode(AVFrame *pFrame, int index, int type) {


    AVCodecContext *pCodecCtx = outputFormat->streams[openedCodec[index]]->codec;
    AVPacket *pkt = av_packet_alloc();
    int got_picture = 0;
    int ret = 0;
    if (type == 0) {
        ret = avcodec_encode_video2(pCodecCtx, pkt, pFrame, &got_picture);
    } else {
        ret = avcodec_encode_audio2(pCodecCtx, pkt, pFrame, &got_picture);
    }
    if (ret < 0) {
        av_packet_free(&pkt);
        loge("Failed to encode：%s\n", av_err2str(ret));
        return;
    }

    if (got_picture == 1) {
        pkt->stream_index = openedCodec[index];
        int re = 0;
        if ((re = av_write_frame(outputFormat, pkt)) < 0) {
            loge("av_write_frame video fail,errorCode=%d,info=%s", re, av_err2str(re));
        } else {
//            logd("写帧数据成功");
        }
        av_packet_free(&pkt);
    } else {
        av_packet_free(&pkt);
    }
}


int Encoder::writeHeader() {
    int timescale = 0;
    for (int i = 0; i < outputFormat->nb_streams; i++) {
        if (outputFormat->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            timescale = outputFormat->streams[i]->time_base.den;
            break;
        }
    }
    AVDictionary *opt = NULL;
    av_dict_set_int(&opt, "video_track_timescale", timescale, 0);  //加入该参数后 解决帧率问题
    return avformat_write_header(outputFormat, &opt);
}

int Encoder::writeTrailer() {
    return av_write_trailer(outputFormat);
}

bool Encoder::IsPrepared() const {
    return isPrepared;
}

void Encoder::release() {
    auto iter = openedCodec.begin();
    while (iter != openedCodec.end()) {
        avcodec_close(outputFormat->streams[iter->second]->codec);
        iter++;
    }
    openedCodec.clear();
    if (outputFormat != nullptr) {
        avformat_close_input(&outputFormat);
        outputFormat = nullptr;
    }
}

Encoder::~Encoder() {
    release();
}


