//
// Created by ZMY on 2019/2/25.
//

#ifndef APP_PACKETPRODUCER_H
#define APP_PACKETPRODUCER_H
struct MediaData {
    int width;
    int height;
    double frame_rate;
    int nb_video_frame;
    int nb_audio_frame;
};

extern "C" {
#include <FFmpeg/avformat.h>
}

#include <vector>
#include <algorithm>
#include <pthread.h>
#include "common_utils.h"
#include "OnProducePakcetListener.h"
#include <map>
#include <list>


using namespace std;

class PacketProducer {
private:
    char *dataSource = nullptr;
    int videoStreamIndex = -1;
    bool running = false;
    vector<int> audioStreamsIndexList;
    AVFormatContext *formatCtx = nullptr;
    OnProducePakcetListener *listener = nullptr;
    vector<int> openedCodecs;
    int needStreamIndex = -1;
    bool autoRelease = true;

    bool initCodec(int index);

    bool isPrepared = false;
    pthread_t thread;
public:

    bool isAutoRelease() const;

    void setAutoRelease(bool autoRelease);

    PacketProducer();

    bool getIsPrepared() const;

    char *getDataSource() const;

    void setDataSource(char *dataSource);

    int findStream();

    static void *doProducePacket(void *context);

    int initDataSource();

    void producePacket();

    void producePacketSync();

    OnProducePakcetListener *getListener() const;

    void setListener(OnProducePakcetListener *listener);

    AVFormatContext *getFormatCtx() const;

    int getVideoStreamIndex() const;

    const vector<int> &getAudioStreamsIndexList() const;

    bool isRunning() const;

    void setRunning(bool running);

    AVCodecContext *getCodecStruct(int index);

    virtual ~PacketProducer();

    void release();

    int getNeedStreamIndex() const;

    void setNeedStreamIndex(int needStreamIndex);

};

#endif //APP_PACKETPRODUCER_H
