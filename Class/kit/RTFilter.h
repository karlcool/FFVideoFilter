//
//  RTFilter.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/12.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AVFrameTool.h"

NS_ASSUME_NONNULL_BEGIN

@interface RTFilter : NSObject

@property(nonatomic, readonly)NSString *desc;

@property(nonatomic, readonly)int width;

@property(nonatomic, readonly)int height;

- (nonnull instancetype)initWithDesc: (NSString*)desc width: (int)width height: (int)height;

- (CVPixelBufferRef)handleBuffer: (CVPixelBufferRef)buffer;

@end

NS_ASSUME_NONNULL_END
