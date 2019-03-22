//
//  VideoInfo.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/21.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface VideoInfo : NSObject {
    CGAffineTransform _transform;
    int _angle;
    int _width;
    int _height;
    NSString *_transpose;//FFMpeg专用
}

- (nonnull instancetype)initWithURL: (NSURL*)url;

@property(nonatomic, readonly)CGAffineTransform transform;

@property(nonatomic, readonly)int angle;

@property(nonatomic, readonly)int width;

@property(nonatomic, readonly)int height;

@property(nonatomic, readonly)NSString *transpose;

+ (int)videoAngle: (CGAffineTransform)transform;

@end

NS_ASSUME_NONNULL_END
