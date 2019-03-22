//
// Created by ZMY on 2019/2/25.
//


#include "PacketProducer.h"

using namespace std;

char *PacketProducer::getDataSource() const {
    return dataSource;
}

void PacketProducer::setDataSource(char *dataSource) {
    PacketProducer::dataSource = dataSource;
    release();
    if (initDataSource() == 0 &&
        findStream() == 0) {
        av_dump_format(formatCtx, audioStreamsIndexList.front(), "", 0);

        isPrepared = true;
    }
}

/**
 * 轮询找到视频中的各个流（视频流，音频流），由于视频可以能有多条音轨，所有音频流用vector来保存
 */
int PacketProducer::findStream() {
    audioStreamsIndexList.clear();
    //遍历所有类型的流（音频流、视频流）
    for (int i = 0; i < formatCtx->nb_streams; i++) {

        if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            logd("找到视频流");
            videoStreamIndex = i;
        } else if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            logd("找到音频流");
            audioStreamsIndexList.push_back(i);
        }
    }
    if (videoStreamIndex == -1) {
        loge("找不到视频流");
        return -1;
    }
    return 0;
}

/**
 * 初始化视频文件   拿到AVFormatContext
 * @return
 */
int PacketProducer::initDataSource() {
    int result = avformat_open_input(&formatCtx, dataSource, NULL, NULL);//打开视频文件
    if (result != 0) {
        loge("文件打开失败");
        return result;
    }
    result = avformat_find_stream_info(formatCtx, NULL);
    if (result < 0) {
        loge("获取文件信息失败");
        return result;
    }
    running = true;
    return 0;
}


void PacketProducer::producePacket() {
    pthread_create(&thread, nullptr, doProducePacket, this);
}

void PacketProducer::producePacketSync() {
    doProducePacket(this);
}

//轮询从文件中读取每一帧那所文件AVPacket  并回调出去
void *PacketProducer::doProducePacket(void *context) {
    PacketProducer *producer = static_cast<PacketProducer *>(context);
    if (producer->listener != nullptr) {
        producer->listener->onStart();
    }
    producer->running = true;
    while (producer->running) {
        AVPacket *packet = av_packet_alloc();
        if (av_read_frame(producer->formatCtx, packet) < 0) {
            av_packet_free(&packet);
            break;
        }
        if (producer->listener == nullptr ||
            (producer->needStreamIndex >= 0 &&
             packet->stream_index != producer->needStreamIndex) ||
            !(producer->listener->onProducePacket(packet))) {
            av_packet_free(&packet);
        }
    }
    producer->running = false;
    if (producer->listener != nullptr) {
        producer->listener->onEnd();
    }
    if (producer->autoRelease) {
        producer->release();
    }
    return nullptr;
}


OnProducePakcetListener *PacketProducer::getListener() const {
    return listener;
}

void PacketProducer::setListener(OnProducePakcetListener *listener) {
    PacketProducer::listener = listener;
}

AVFormatContext *PacketProducer::getFormatCtx() const {
    return formatCtx;
}

int PacketProducer::getVideoStreamIndex() const {
    return videoStreamIndex;
}

const vector<int> &PacketProducer::getAudioStreamsIndexList() const {
    return audioStreamsIndexList;
}

bool PacketProducer::isRunning() const {
    return running;
}

void PacketProducer::setRunning(bool running) {
    PacketProducer::running = running;
}

AVCodecContext *PacketProducer::getCodecStruct(int index) {
    if (index < 0 || index >= formatCtx->nb_streams) {
        return NULL;
    }
    if (find(openedCodecs.begin(), openedCodecs.end(), index) == openedCodecs.end()) {
        if (initCodec(index)) {
            openedCodecs.push_back(index);
        } else {
            return nullptr;
        }
    }
    return formatCtx->streams[index]->codec;
}

bool PacketProducer::initCodec(int index) {     //打开对应位置流（音频或视频）的解码器
    AVCodecContext *codecContext = formatCtx->streams[index]->codec;
    if (index == videoStreamIndex) {
        codecContext->thread_count = 8;  //TODO 线程数待定
    }
    AVCodec *avCodec = avcodec_find_decoder(codecContext->codec_id);

    if (avCodec == NULL) {
        loge("找不到解码器");
        return false;
    }
    int result = avcodec_open2(codecContext, avCodec, NULL);
    if (result < 0) {
        loge("打开解码器失败");
        return false;
    }
    logd("解码器的名称：%s", avCodec->name);
    return true;
}

PacketProducer::~PacketProducer() {
    release();
}

bool PacketProducer::getIsPrepared() const {
    return isPrepared;
}

void PacketProducer::release() {
    running = false;
    isPrepared = false;
    audioStreamsIndexList.clear();
    for (int i = 0; i < openedCodecs.size(); i++) {
        avcodec_close(formatCtx->streams[openedCodecs[i]]->codec);
    }
    openedCodecs.clear();
    if (formatCtx != nullptr) {
        avformat_close_input(&formatCtx);
        formatCtx = nullptr;
    }
}

PacketProducer::PacketProducer() {
    av_register_all();
    av_log_set_callback(log_callback_test2);
}

int PacketProducer::getNeedStreamIndex() const {
    return needStreamIndex;
}

void PacketProducer::setNeedStreamIndex(int needStreamIndex) {
    PacketProducer::needStreamIndex = needStreamIndex;
}

bool PacketProducer::isAutoRelease() const {
    return autoRelease;
}

void PacketProducer::setAutoRelease(bool autoRelease) {
    PacketProducer::autoRelease = autoRelease;
}







