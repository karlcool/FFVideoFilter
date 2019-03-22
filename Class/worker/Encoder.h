//
// Created by ZMY on 2019/2/25.
//

#ifndef APP_ENCODER_H
#define APP_ENCODER_H


extern "C" {
#include <FFmpeg/avformat.h>
#include <FFmpeg/avcodec.h>
#include <FFmpeg/opt.h>
};

#include <vector>
#include <map>

using namespace std;

class Encoder {
private:
    AVFormatContext *outputFormat = nullptr;
    char *outputFile = nullptr;
    map<int, int> openedCodec; //key:对应输入文件的各个流index    value:对应输出各个流的index
    bool isPrepared = true;

    void release();

public:
    Encoder(char *outputFile);

    char *getOutputFile() const;

    void setOutputFile(char *outputFile);


    bool init(AVFormatContext *fmt, int index, int type, bool needOpenCodec,
              map<char *, char *> *option);

    void encode(AVFrame *, int index, int type);

    int writeHeader();

    int writeTrailer();

    bool IsPrepared() const;

    virtual ~Encoder();

};


#endif //APP_ENCODER_H
