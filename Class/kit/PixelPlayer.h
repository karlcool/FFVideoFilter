//
//  PixelPlayer.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, LRVideoStatus) {
    LRVideoStatusUnknow,
    LRVideoStatusFailed,
    LRVideoStatusPrepare,
    LRVideoStatusPlaying,
    LRVideoStatusPause,
    LRVideoStatusFinished,
};

@class PixelPlayer;
@protocol PixelPlayerDelegate <NSObject>

- (nullable CVPixelBufferRef)pixelPlayer: (PixelPlayer*)player displayBuffer:(CVPixelBufferRef)buffer;

@end

@interface PixelPlayer : UIImageView {
    CADisplayLink *displayLink;
}

@property(nonatomic, weak)id<PixelPlayerDelegate> delegate;

@property(nonatomic, strong)AVSampleBufferDisplayLayer *displayLayer;

@property(nonatomic, strong)AVPlayerItem *playItem;

@property(nonatomic, strong)AVPlayerItemVideoOutput *output;

@property(nonatomic, strong)AVPlayer *player;

@property(nonatomic, assign)LRVideoStatus status;

@property(nonatomic, assign)BOOL isPlaying;

@property(nonatomic, readonly)CGFloat progress;

@property(nonatomic, readonly)CGFloat duration;

@property(nonatomic, readonly)CGAffineTransform videoTransform;

@property(nonatomic, assign)BOOL isLoop;

@property(nonatomic, copy)void (^progressChanged)(CGFloat progress);

- (nonnull instancetype)initWithURL: (NSURL*)url;

- (void)play;

- (void)stop: (BOOL)isFinish;

@end

NS_ASSUME_NONNULL_END
