//
//  VideoInfo.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/21.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "VideoInfo.h"
#import <AVFoundation/AVFoundation.h>

@implementation VideoInfo

- (nonnull instancetype)initWithURL: (NSURL*)url {
    self = [super init];
    if (self) {
        AVPlayerItem *item = [[AVPlayerItem alloc] initWithURL:url];

        AVAssetTrack *track = [[[item asset] tracksWithMediaType:AVMediaTypeVideo] firstObject];
        _transform = [track preferredTransform];
        _angle = [VideoInfo videoAngle:_transform];
        
        _width = track.naturalSize.width;
        _height = track.naturalSize.height;
        [self setupTranspose];
    }
    return self;
}

- (CGAffineTransform)transform {
    return _transform;
}

- (int)angle {
    return _angle;
}

- (int)width {
    return _width;
}

- (int)height {
    return _height;
}

- (NSString*)transpose {
    return _transpose;
}

- (void)setupTranspose {
    //transpose={0,1,2,3} 0:逆时针旋转90°然后垂直翻转 1:顺时针旋转90° 2:逆时针旋转90° 3:顺时针旋转90°然后水平翻转
    if (_angle == 0) {
        _transpose = @"";
    } else if (_angle == 90) {
        _transpose = @",transpose=2";
    } else if (_angle == 180) {
        _transpose = @",transpose=2,transpose=2";
    } else if (_angle == 270) {
        _transpose = @",transpose=1";
    }
}

+ (int)videoAngle: (CGAffineTransform)transform {
    int angle = 0;
    if (transform.a == 0 && transform.b == 1.0 && transform.c == -1.0 && transform.d == 0) {
        angle = 90;// Portrait
    } else if (transform.a == 0 && transform.b == -1.0 && transform.c == 1.0 && transform.d == 0) {
        angle = 270;// PortraitUpsideDown
    } else if (transform.a == 1.0 && transform.b == 0 && transform.c == 0 && transform.d == 1.0) {
        angle = 0;// LandscapeRight
    } else if (transform.a == -1.0 && transform.b == 0 && transform.c == 0 && transform.d == -1.0) {
        angle = 180;// LandscapeLeft
    }
    return angle;
}

@end
