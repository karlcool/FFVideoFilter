//
// Created by ZMY on 2018/9/29.
//

#ifndef MEDIAPLAYER_COMMON_UTILS_H
#define MEDIAPLAYER_COMMON_UTILS_H

#include <string>

extern "C" {
#include <FFmpeg/log.h>
};

#define DUBUG 1
#define logd(fmt, ...) if(DUBUG)
#define loge(fmt, ...) if(DUBUG)
#define logi(fmt, ...) if(DUBUG)

//#define logd(...) printf(__VA_ARGS__)
//#define loge(...) printf(__VA_ARGS__)
#define SHOW_FFMPEG_LOG 0


static void log_callback_test2(void *ptr, int level, const char *fmt, va_list vl) {
//    if (SHOW_FFMPEG_LOG) {
//        va_list vl2;
//        char *line = static_cast<char *>(malloc(128 * sizeof(char)));
//        static int print_prefix = 1;
//        va_copy(vl2, vl);
//        av_log_format_line(ptr, level, fmt, vl2, line, 128, &print_prefix);
//        va_end(vl2);
//        line[127] = '\0';
//        __android_log_print("INFO", "ffm_native_log", "%s", line);
//        free(line);
//    }
}

#endif //MEDIAPLAYER_COMMON_UTILS_H
