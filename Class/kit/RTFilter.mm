//
//  RTFilter.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/12.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "RTFilter.h"
#include "Filter.h"

@interface RTFilter () {
    NSString *_desc;
    int _width;
    int _height;
    
    AVFrameTool *frameTool;
    Filter *coreFilter;
}

@end

@implementation RTFilter

- (nonnull instancetype)initWithDesc: (NSString*)desc width: (int)width height: (int)height {
    self = [super init];
    if (self) {
        _desc = desc;
        _width = width;
        _height = height;
        
        frameTool = [[AVFrameTool alloc] init];
        coreFilter = new Filter(width, height, AV_PIX_FMT_YUV420P, {1, 1}, {0, 1}, const_cast<char *>(desc.UTF8String));
    }
    return self;
}

- (int)width {
    return _width;
}

- (int)height {
    return _height;
}

- (NSString*)desc {
    return _desc;
}

- (void)dealloc {
    delete coreFilter;
}

- (CVPixelBufferRef)handleBuffer: (CVPixelBufferRef)buffer {
    AVFrame *sourceFrame = (AVFrame *)[frameTool avFrameFromPixelBuffer:buffer];

    void *oldData = sourceFrame->data[0];
    coreFilter->doWork(sourceFrame);
    void *newData = sourceFrame->data[0];

    CVPixelBufferRef resultBuffer = [frameTool pixelBufferFromAVFrame:sourceFrame];

    av_free(oldData);
    av_free(newData);
    return resultBuffer;
}

@end
