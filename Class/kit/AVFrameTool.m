//
//  AVFrameTool.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "AVFrameTool.h"
#import <FFmpeg/imgutils.h>
#import <FFmpeg/frame.h>
#import <FFmpeg/swscale.h>
@implementation AVFrameTool

- (void)dealloc {
    CVPixelBufferPoolRelease(_pixelBufferPool);
}

#pragma mark CVPixelBufferRef和AVFrame互转
- (void*)avFrameFromPixelBuffer: (CVPixelBufferRef)buffer {
    CVPixelBufferLockBaseAddress(buffer, 0);
    
    int width = (int)CVPixelBufferGetWidth(buffer);
    int height = (int)CVPixelBufferGetHeight(buffer);
    unsigned char *rawPixelBase = (unsigned char *)CVPixelBufferGetBaseAddress(buffer);
    
    AVFrame *pFrame = av_frame_alloc();
    av_image_fill_arrays(pFrame->data, pFrame->linesize, rawPixelBase, AV_PIX_FMT_BGRA, width, height, 1);
    pFrame->width = width;
    pFrame->height = height;
    pFrame->format = AV_PIX_FMT_BGRA;
    
    CVPixelBufferUnlockBaseAddress(buffer, 0);
    
    AVFrame *tempFrame = [self convert:pFrame toFormat:AV_PIX_FMT_YUV420P];
    
    av_frame_free(&pFrame);
    return tempFrame;
}

- (CVPixelBufferRef)pixelBufferFromAVFrame: (void*)frame {
    if(!((AVFrame*)frame) || !((AVFrame*)frame)->data[0]){
        return nil;
    }
    AVFrame *tempFrame = [self convert:((AVFrame*)frame) toFormat:AV_PIX_FMT_BGRA];
  
    CVReturn theError;
    if (!_pixelBufferPool) {
        NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
        [attributes setObject:@(kCVPixelFormatType_32BGRA) forKey:(NSString*)kCVPixelBufferPixelFormatTypeKey];
        [attributes setObject:@(tempFrame->width) forKey: (NSString*)kCVPixelBufferWidthKey];
        [attributes setObject:@(tempFrame->height) forKey: (NSString*)kCVPixelBufferHeightKey];
        [attributes setObject:@(tempFrame->linesize[0]) forKey:(NSString*)kCVPixelBufferBytesPerRowAlignmentKey];
        [attributes setObject:@{} forKey:(NSString*)kCVPixelBufferIOSurfacePropertiesKey];
        theError = CVPixelBufferPoolCreate(kCFAllocatorDefault, NULL, (__bridge CFDictionaryRef) attributes, &_pixelBufferPool);
    }
    
    CVPixelBufferRef pixelBuffer = nil;
    theError = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, _pixelBufferPool, &pixelBuffer);
 
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    size_t bytePerRowY = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);
    
    void *base = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    memcpy(base, tempFrame->data[0], bytePerRowY * tempFrame->height);
    
    av_free(tempFrame->data[0]);
    av_frame_free(&tempFrame);
    return pixelBuffer;
}

- (void*)avFrameFromImage: (UIImage*)image {
    CGImageRef cgImage = image.CGImage;
    if (!cgImage) {
        return nil;
    }
    
    NSDictionary *options = @{(NSString*)kCVPixelBufferCGImageCompatibilityKey : @YES,
                              (NSString*)kCVPixelBufferCGBitmapContextCompatibilityKey : @YES,
                              (NSString*)kCVPixelBufferIOSurfacePropertiesKey: [NSDictionary dictionary]};
    CVPixelBufferRef pxbuffer = NULL;
    
    CGFloat frameWidth = CGImageGetWidth(cgImage);
    CGFloat frameHeight = CGImageGetHeight(cgImage);
    
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault,
                                          frameWidth,
                                          frameHeight,
                                          kCVPixelFormatType_32BGRA,
                                          (__bridge CFDictionaryRef) options,
                                          &pxbuffer);
    
    NSParameterAssert(status == kCVReturnSuccess && pxbuffer != NULL);
    
    CVPixelBufferLockBaseAddress(pxbuffer, 0);
    void *pxdata = CVPixelBufferGetBaseAddress(pxbuffer);
    NSParameterAssert(pxdata != NULL);
    
    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    
    CGContextRef context = CGBitmapContextCreate(pxdata,
                                                 frameWidth,
                                                 frameHeight,
                                                 8,
                                                 CVPixelBufferGetBytesPerRow(pxbuffer),
                                                 rgbColorSpace,
                                                 (CGBitmapInfo)kCGImageAlphaNoneSkipFirst);
    NSParameterAssert(context);
    CGContextConcatCTM(context, CGAffineTransformIdentity);
    CGContextDrawImage(context, CGRectMake(0,
                                           0,
                                           frameWidth,
                                           frameHeight),
                       cgImage);
    CGColorSpaceRelease(rgbColorSpace);
    CGContextRelease(context);
    
    CVPixelBufferUnlockBaseAddress(pxbuffer, 0);
    
    AVFrame *result = [self avFrameFromPixelBuffer:pxbuffer];
    CVBufferRelease(pxbuffer);
    return result;
}

- (UIImage*)imageFromAVFrame: (void*)frame {
    CVImageBufferRef imageBuffer = [self pixelBufferFromAVFrame:frame];
    
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    void *baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    size_t bufferSize = CVPixelBufferGetDataSize(imageBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 0);
    
    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, baseAddress, bufferSize, NULL);
    
    CGImageRef cgImage = CGImageCreate(width, height, 8, 32, bytesPerRow, rgbColorSpace, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrderDefault, provider, NULL, true, kCGRenderingIntentDefault);
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    CGImageRelease(cgImage);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(rgbColorSpace);
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    return image;
}

- (AVFrame*)convert: (AVFrame*)sFrame toFormat: (enum AVPixelFormat)toFormat {
    int numBytes = av_image_get_buffer_size(toFormat, sFrame->width, sFrame->height, 1);
    
    struct SwsContext *sws_ctx = sws_getContext(sFrame->width, sFrame->height, sFrame->format, sFrame->width, sFrame->height, toFormat, SWS_BICUBIC, NULL, NULL, NULL);
    
    AVFrame *resultFrame = av_frame_alloc();
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(resultFrame->data, resultFrame->linesize, buffer, toFormat, sFrame->width, sFrame->height, 1);
    
    int ret = sws_scale(sws_ctx, (uint8_t const *const *)sFrame->data, sFrame->linesize, 0, sFrame->height, resultFrame->data, resultFrame->linesize);
    if (ret < 0) {
        av_frame_free(&resultFrame);
        return nil;
    }
    resultFrame->width = sFrame->width;
    resultFrame->height = sFrame->height;
    resultFrame->best_effort_timestamp = sFrame->best_effort_timestamp;
    resultFrame->pkt_dts = sFrame->pkt_dts;
    resultFrame->pkt_duration = sFrame->pkt_duration;
    resultFrame->format = toFormat;
    
    sws_freeContext(sws_ctx);
    return resultFrame;
}

@end
