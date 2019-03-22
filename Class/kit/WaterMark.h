//
//  WaterMark.h
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface WaterMark : NSObject

@property(nonatomic, copy)NSString *path;
@property(nonatomic, assign)CGRect rect;

- (instancetype)initWithPath: (NSString*)path rect: (CGRect)rect;

@end

NS_ASSUME_NONNULL_END
