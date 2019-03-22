//
// Created by ZMY on 2019/2/26.
//

#include <sys/time.h>
#include "Transformer.h"
#include "Encoder.h"


Transformer::Transformer() {
    packetProducer = new PacketProducer();
    packetProducer->setListener(this);
}

Transformer::Transformer(char *dataSource) : dataSource(dataSource) {
    Transformer();
    setDataSource(dataSource);
}

char *Transformer::getDataSource() const {
    return packetProducer->getDataSource();
}

void Transformer::setDataSource(char *dataSource) {
    //判断参数是否完备
    if (dataSource == nullptr || outputFile == nullptr) {
        isPrepared = false;
        return;
    }
    //设置源视频路径
    packetProducer->setDataSource(dataSource);
    if (!packetProducer->getIsPrepared()) {
        isPrepared = false;
        return;
    }
    //
    AVCodecContext *codec = packetProducer->getCodecStruct(packetProducer->getVideoStreamIndex());
    if (codec == nullptr) {
        isPrepared = false;
        return;
    }
    videoDecoder = new Decoder(codec);
    videoDecoder->setListener(this);

    AVCodecContext *audioCodec = packetProducer->getCodecStruct(
            packetProducer->getAudioStreamsIndexList().front());
    if (audioCodec == nullptr) {
        isPrepared = false;
        return;
    }
    audioDecoder = new Decoder(audioCodec);
    audioDecoder->setListener(this);

    if (filterDescription) {
        setFilter(filterDescription);
    }
    if (filterDescription && !filter->getIsPrepared()) {
        isPrepared = false;
        return;
    }
    encoder = new Encoder(outputFile);
    if (!encoder->IsPrepared()) {
        isPrepared = false;
        return;
    }
    for (int i = 0; i < packetProducer->getFormatCtx()->nb_streams; i++) {
        int type = 0;
        bool needEncode;
        map<char *, char *> option;
        if (i == packetProducer->getVideoStreamIndex()) {
            type = 0;
            needEncode = true;
            AVDictionary *metadata = packetProducer->getFormatCtx()->streams[packetProducer->getVideoStreamIndex()]->metadata;
            AVDictionaryEntry *data = av_dict_get(metadata, "rotate", nullptr, 0);
            if (data) {
                option[data->key] = data->value;
            }
        } else {
            type = 1;
            needEncode = true;
        }
        if (!encoder->init(packetProducer->getFormatCtx(),
                           i, type, needEncode, &option)) {
            isPrepared = false;
            return;
        }
    }
}

void Transformer::start() {
    if (isPrepared) {
        packetProducer->producePacketSync();  //开始读取视频文件中每一帧的压缩文件 AVPacket
    }
}

bool Transformer::onProducePacket(AVPacket *packet) {
    if (packet->stream_index == packetProducer->getVideoStreamIndex()) {
        videoDecoder->decode(packet, packet->stream_index, 0); //解码视频帧
    } else if (packet->stream_index == packetProducer->getAudioStreamsIndexList().front()) {
        audioDecoder->decode(packet, packet->stream_index, 1);//解码音频帧
    }
    return false;
}

bool Transformer::onDecodeFrame(AVFrame *frame, int index, int type) {   //解码出视频帧的回调
    if (type == 0) {
        if (filter) {
            filter->doWork(frame);//为视频帧滤镜
        }
        encoder->encode(frame, index, type);//重新编码并写入文件
    } else if (type == 1) {
        if (!keepAuido) {
            AVFrame *silenceFrame = alloc_silence_frame(frame);
            encoder->encode(silenceFrame, index, type);
            av_frame_free(&silenceFrame);
        } else {
            encoder->encode(frame, index, type);
        }

    }
    return false;
}

AVFrame *
Transformer::alloc_silence_frame(AVFrame *input) {
    AVFrame *frame;
    int32_t ret;
    frame = av_frame_alloc();
    if (!frame) {
        return nullptr;
    }

    frame->sample_rate = input->sample_rate;
    frame->format = input->format; /*默认的format:AV_SAMPLE_FMT_FLTP*/
    frame->channel_layout = av_get_default_channel_layout(input->channels);
    frame->channels = input->channels;
    frame->nb_samples = input->nb_samples; /*默认的sample大小:1024*/
    frame->pts = input->pts;
    frame->pkt_pts = input->pkt_pts;
    frame->best_effort_timestamp = input->best_effort_timestamp;
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        av_frame_free(&frame);
        return nullptr;
    }

    av_samples_set_silence(frame->data, 0, frame->nb_samples, frame->channels,
                           static_cast<AVSampleFormat>(frame->format));
    return frame;
}

void Transformer::onStart() {  //获取压缩数据开始
    if (encoder->writeHeader() < 0) {
        loge("write header info error");
        isPrepared = false;
    }
    logd("onStart");
}

void Transformer::onEnd() {  //获取压缩数据结束（到文件尾了）
    logd("onEnd");
    if (encoder->writeTrailer() < 0) {
        loge("write Trailer info error");
        isPrepared = false;
    }
    release();
}

char *Transformer::getFilterDescription() const {
    return filterDescription;
}

void Transformer::setFilterDescription(char *filterDescription) {
    Transformer::filterDescription = filterDescription;
}

char *Transformer::getOutputFile() const {
    return outputFile;
}

void Transformer::setOutputFile(char *outputFile) {
    Transformer::outputFile = outputFile;
}

void Transformer::release() {
    if (videoDecoder) {
        delete (videoDecoder);
        videoDecoder = nullptr;
    }
    if (audioDecoder) {
        delete (audioDecoder);
        audioDecoder = nullptr;
    }
    if (encoder) {
        delete (encoder);
        encoder = nullptr;
    }
    if (filter) {
        delete (filter);
        filter = nullptr;
    }
    if (packetProducer) {
        delete (packetProducer);
        packetProducer = nullptr;
    }
}

Transformer::~Transformer() {
    release();
}

bool Transformer::isKeepAuido() const {
    return keepAuido;
}

void Transformer::setKeepAuido(bool keepAuido) {
    Transformer::keepAuido = keepAuido;
}

void Transformer::stop() {
    packetProducer->setRunning(false);
}

void Transformer::setFilter(char *filterStr) {
    if (filter) {
        delete (filter);
        filter = nullptr;
    }
    this->filterDescription = filterStr;
    filter = new Filter(packetProducer->getCodecStruct(packetProducer->getVideoStreamIndex()),
                        filterDescription);
}

bool Transformer::IsPrepared() const {
    return isPrepared;
}

void Transformer::setIsPrepared(bool isPrepared) {
    Transformer::isPrepared = isPrepared;
}


