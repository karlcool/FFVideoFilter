//
//  FFMpegTool.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "FFMpegTool.h"
#include "Transformer.h"

@interface FFMpegTool ()

@end

@implementation FFMpegTool

- (nonnull instancetype)initWithIn: (NSString*)inFile out: (NSString*)outFile info: (VideoInfo*)info {
    self = [super init];
    if (self) {
        _inFile = inFile;
        _outFile = outFile;
        _info = info;
    }
    return self;
}

- (NSString*)inFile {
    return _inFile;
}

- (NSString*)outFile {
    return _outFile;
}

#pragma mark 向视频添加水印
- (void)doTranslateWithWaterMarks: (nullable NSArray<WaterMark*>*)waterMarks filter: (nullable RTFilter*)filter keepAuido: (BOOL)keepAuido completion:(void(^)(BOOL succeed, NSString *outPath))completion {
    NSMutableString *desc = [NSMutableString string];
    NSString *inTag = @"[in]";
    if (filter) {
        NSString *filterDesc = filter.desc;
        if (waterMarks.count > 0) {
            filterDesc = [filter.desc stringByReplacingOccurrencesOfString:@"[out]" withString:@"[filter];"];
        }
        [desc appendString:filterDesc];
        inTag = @"[filter]";
    }

    int i = 0;
    for (WaterMark *wm in waterMarks) {
        CGRect rect = [self rectInVideo:wm.rect];
        if (i==0) {
            [desc appendFormat:@"movie=%@,scale=%i:%i%@[wm%i];%@[wm%i]overlay=%i:%i", wm.path, (int)rect.size.width, (int)rect.size.height, _info.transpose, i, inTag, i, (int)rect.origin.x, (int)rect.origin.y];
        } else {
            [desc appendString:@"[result];"];
            [desc appendFormat:@"movie=%@,scale=%i:%i%@[wm%i];[result][wm%i]overlay=%i:%i", wm.path, (int)rect.size.width, (int)rect.size.height, _info.transpose, i, i, (int)rect.origin.x, (int)rect.origin.y];
        }
        i++;
    }
    [desc appendString:@"[out]"];
    NSLog(@"%@", desc);

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        Transformer *transformer = new Transformer();
        transformer->setFilterDescription(const_cast<char *>(desc.UTF8String));
        transformer->setOutputFile(const_cast<char *>(self->_outFile.UTF8String));
        transformer->setKeepAuido(keepAuido);
        transformer->setDataSource(const_cast<char *>(self->_inFile.UTF8String));
        transformer->start();
        
        BOOL result = transformer->IsPrepared();
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(result, self->_outFile);
        });
    });
}

- (CGRect)rectInVideo:(CGRect)origin {
    CGFloat scale = UIScreen.mainScreen.scale;
    CGFloat newX = origin.origin.x * scale;
    CGFloat newY = origin.origin.y * scale;
    CGFloat newWidth = origin.size.width * scale;
    CGFloat newHeight = origin.size.height * scale;
    
    if (_info.angle == 0) {
        return CGRectMake(newX, newY, newWidth, newHeight);
    } else if (_info.angle == 90) {
        return CGRectMake(newY, _info.height - newWidth - newX, newWidth, newHeight);
    } else if (_info.angle == 180) {
        return CGRectMake(_info.width - newWidth - newX, _info.height - newHeight - newY, newWidth, newHeight);
    } else if (_info.angle == 270) {
        return CGRectMake(_info.width - newHeight - newY, newX, newWidth, newHeight);
    }
    
    return origin;
}

@end
