//
//  PixelPlayer.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "PixelPlayer.h"
#import "RTFilter.h"

@interface PixelPlayer () {
    BOOL isSliding;//是否正在拖动
    BOOL isPlayed;//是否播放到中途
    BOOL isStoped;//是否已经停止播放
    BOOL haveObserver;//是否监听播放器状态
    id timeObserver;//是否监听播放进度
    
    CGFloat _progress;
    CGFloat _duration;
    CGAffineTransform _videoTransform;
}

@end

@implementation PixelPlayer

- (nonnull instancetype)initWithURL: (NSURL*)url {
    self = [super init];
    if (self) {
        self.userInteractionEnabled = YES;
        
        _output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:@{(__bridge id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)}];
        _playItem = [[AVPlayerItem alloc] initWithURL:url];
        [_playItem addOutput:_output];
        _player = [[AVPlayer alloc] initWithPlayerItem:_playItem];
 
        _videoTransform = [[[[_playItem asset] tracksWithMediaType:AVMediaTypeVideo] firstObject] preferredTransform];
        [self setupSampleBufferDisplayLayer];
    }
    return self;
}

- (void)dealloc {
    [self removeObservers];
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    _displayLayer.frame = self.bounds;
}

#pragma mark - 播放控制
- (void)play {
    if (isPlayed) {
        [_player play];
        isStoped = NO;
        _isPlaying = YES;
        return;
    }
    isPlayed = YES;
    haveObserver = YES;
    
    [_player play];

    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(playCompletion) name:AVPlayerItemDidPlayToEndTimeNotification object:nil];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(pause) name:UIApplicationWillResignActiveNotification object:nil];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(play) name:UIApplicationDidBecomeActiveNotification object:nil];
    [_player.currentItem addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew context:nil];
    
    CGFloat time = [_player.currentItem currentTime].value;
    if (time > 0.001) {
        CMTime changedTime = CMTimeMake(0, 1);
        __weak PixelPlayer *weakSelf = self;
        [_player.currentItem seekToTime:changedTime completionHandler:^(BOOL finished) {
            [weakSelf.player play];
        }];
    } else {
        [_player play];
    }
    
    if (!timeObserver) {
        [self monitoringPlayback:_playItem];
    }
    
    isStoped = NO;
    _isPlaying = YES;
    _status = LRVideoStatusPlaying;
    
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(refreshDisplay:)];
    [displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
}

- (void)pause {
    [_player pause];
    _isPlaying = NO;
    _status = LRVideoStatusPause;
}

- (void)stop: (BOOL)isFinish {
    if (isStoped) {
        return;
    }
    isStoped = YES;
    _isPlaying = NO;
    isPlayed = NO;
    
    [_player pause];
    [self removeObservers];
    if (isFinish) {
        _status = LRVideoStatusFinished;
    }
}

- (void)replay {
    [self stop:NO];
    [self play];
}

//MARK: 拖动进度
- (void)setPlayProgress: (CGFloat)progress {
    isSliding = YES;
    [self pause];
    CMTime changedTime = CMTimeMake(progress, 1);
    __weak PixelPlayer *weakSelf = self;
    [_player.currentItem seekToTime:changedTime completionHandler:^(BOOL finished) {
        self->isSliding = NO;
        [weakSelf play];
    }];
}

- (void)playCompletion {
    if (_isLoop) {
        [self replay];
    } else {
        [self stop:YES];
    }
}

- (void)changePlayState {
    if (_isPlaying) {
        [self pause];
    } else {
        [self play];
    }
}

//MARK: 移除所有监听
- (void)removeObservers {
    if (haveObserver) {
        haveObserver = NO;
        [_player.currentItem removeObserver:self forKeyPath:@"status"];
    }
    [_player.currentItem cancelPendingSeeks];
    [_player.currentItem.asset cancelLoading];
    
    if (timeObserver) {
        [_player removeTimeObserver:timeObserver];
        timeObserver = nil;
    }
    [NSNotificationCenter.defaultCenter removeObserver:self];
}

- (CGFloat)progress {
    return _progress;
}

- (CGFloat)duration {
    return _duration;
}

- (CGAffineTransform)videoTransform {
    return _videoTransform;
}

#pragma mark - 逐帧回调与渲染
- (void)refreshDisplay: (CADisplayLink*)link {
    CMTime itemTime = [_output itemTimeForHostTime:CACurrentMediaTime()];
    if (![_output hasNewPixelBufferForItemTime:itemTime]) {
        return;
    }
    CVPixelBufferRef tempBuffer = [_output copyPixelBufferForItemTime:itemTime itemTimeForDisplay:nil];
    
    CVPixelBufferRef resultBuffer = [_delegate pixelPlayer:self displayBuffer:tempBuffer];
    if (!resultBuffer) {
        resultBuffer = tempBuffer;
    }
    [self displayBuffer:resultBuffer];
    
    if (tempBuffer == resultBuffer) {
        CVPixelBufferRelease(tempBuffer);
    } else {
        CVPixelBufferRelease(tempBuffer);
        CVPixelBufferRelease(resultBuffer);
    }
}

- (void)displayBuffer: (CVPixelBufferRef)buffer {
    //不设置具体时间信息
    CMSampleTimingInfo timing = {kCMTimeInvalid, kCMTimeInvalid, kCMTimeInvalid};
    //获取视频信息
    CMVideoFormatDescriptionRef videoInfo = NULL;
    OSStatus result = CMVideoFormatDescriptionCreateForImageBuffer(NULL, buffer, &videoInfo);
    
    CMSampleBufferRef sampleBuffer = NULL;
    result = CMSampleBufferCreateForImageBuffer(kCFAllocatorDefault, buffer, true, NULL, NULL, videoInfo, &timing, &sampleBuffer);
    
    CFRelease(videoInfo);
    
    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, YES);
    CFMutableDictionaryRef dict = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
    CFDictionarySetValue(dict, kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);
    [self enqueueSampleBuffer:sampleBuffer toLayer:_displayLayer];
    CFRelease(sampleBuffer);
}

- (void)enqueueSampleBuffer:(CMSampleBufferRef) sampleBuffer toLayer:(AVSampleBufferDisplayLayer*)layer {
    if (sampleBuffer) {
        CFRetain(sampleBuffer);
        [layer enqueueSampleBuffer:sampleBuffer];
        CFRelease(sampleBuffer);
        if (layer.status == AVQueuedSampleBufferRenderingStatusFailed) {
            [layer flush];
        }
    }
}

- (void)setupSampleBufferDisplayLayer {
    if (!_displayLayer) {
        _displayLayer = [[AVSampleBufferDisplayLayer alloc] init];
        _displayLayer.frame = self.bounds;
        _displayLayer.position = CGPointMake(CGRectGetMidX(self.bounds), CGRectGetMidY(self.bounds));
        _displayLayer.videoGravity = AVLayerVideoGravityResizeAspect;
        _displayLayer.opaque = YES;
        _displayLayer.affineTransform = _videoTransform;
        [self.layer addSublayer:_displayLayer];
    }
}

#pragma mark - 监听播放进度变化
- (void)monitoringPlayback: (AVPlayerItem*)item {
    __weak PixelPlayer *weakSelf = self;
    timeObserver = [_player addPeriodicTimeObserverForInterval:CMTimeMake(1, 60) queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0) usingBlock:^(CMTime time) {
        if (item.currentTime.timescale != 0) {
            CGFloat currentPlayTime = CMTimeGetSeconds(time);
            self->_progress = currentPlayTime;
            if (self->_duration < 0.01) {
                CGFloat second = CMTimeGetSeconds(weakSelf.playItem.duration);
                self->_duration = isnan(second) ? 0.0 : second;
            }
            if (weakSelf.progressChanged) {
                weakSelf.progressChanged(currentPlayTime);
            }
        }
    }];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if ([keyPath isEqualToString:@"status"]) {
        if (_player.status == AVPlayerStatusReadyToPlay) {
            if (_duration < 0.01) {
                CGFloat second = CMTimeGetSeconds(_playItem.duration);
                _duration = isnan(second) ? 0.0 : second;
            }
            
        } else if (_player.status == AVPlayerStatusFailed) {
            _status = LRVideoStatusFailed;
        } else {
            _status = LRVideoStatusUnknow;
        }
    }
}

@end
