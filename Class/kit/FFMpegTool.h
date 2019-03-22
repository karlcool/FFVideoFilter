//
//  FFMpegTool.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "WaterMark.h"
#import "RTFilter.h"
#import "VideoInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface FFMpegTool : NSObject {
    NSString *_inFile;
    NSString *_outFile;
    VideoInfo *_info;
}

@property(nonatomic, readonly)NSString *inFile;

@property(nonatomic, readonly)NSString *outFile;

- (nonnull instancetype)initWithIn: (NSString*)inFile out: (NSString*)outFile info: (VideoInfo*)info;

#pragma mark 向视频添加水印和滤镜
- (void)doTranslateWithWaterMarks: (nullable NSArray<WaterMark*>*)waterMarks filter: (nullable RTFilter*)filter keepAuido: (BOOL)keepAuido completion:(void(^)(BOOL succeed, NSString *outPath))completion;

@end

NS_ASSUME_NONNULL_END
