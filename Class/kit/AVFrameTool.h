//
//  AVFrameTool.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <UIKit/UIKit.h>


NS_ASSUME_NONNULL_BEGIN

@interface AVFrameTool : NSObject

@property(nonatomic)CVPixelBufferPoolRef pixelBufferPool;

- (void*)avFrameFromPixelBuffer: (CVPixelBufferRef)buffer;
- (CVPixelBufferRef)pixelBufferFromAVFrame: (void*)frame;

//- (void*)avFrameFromImage: (UIImage*)image;
//- (UIImage*)imageFromAVFrame: (void*)frame;

@end

NS_ASSUME_NONNULL_END
