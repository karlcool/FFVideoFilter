//
// Created by ZMY on 2019/2/26.
//

#ifndef APP_TRANSFORMER_H
#define APP_TRANSFORMER_H


#include "PacketProducer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Filter.h"

/**
 * 这是视频处理的入口类
 */
class Transformer : public OnProducePakcetListener, OnDecodeFrameListener {
private:
    char *dataSource;
    PacketProducer *packetProducer = nullptr;
    Decoder *videoDecoder = nullptr;
    Decoder *audioDecoder = nullptr;
    Encoder *encoder = nullptr;
    Filter *filter = nullptr;
    bool isPrepared = true;
    char *filterDescription = nullptr;
    char *outputFile = nullptr;
    bool keepAuido = true;  //是否保留音轨  false：生成的新视频将没有任何声音

    void release();

    bool onDecodeFrame(AVFrame *frame, int index, int type) override;

    AVFrame *alloc_silence_frame(AVFrame *input);

public:
    Transformer();

    Transformer(char *dataSource);

    char *getDataSource() const;

    bool IsPrepared() const;

    void setIsPrepared(bool isPrepared);

    void setDataSource(char *dataSource);

    void start();

    bool onProducePacket(AVPacket *packet) override;

    void onStart() override;

    void onEnd() override;

    char *getFilterDescription() const;

    void setFilterDescription(char *filterDescription);

    char *getOutputFile() const;

    void setOutputFile(char *outputFile);

    virtual ~Transformer();

    bool isKeepAuido() const;

    void setKeepAuido(bool keepAuido);

    void stop();

    void setFilter(char *filter);

};


#endif //APP_TRANSFORMER_H
