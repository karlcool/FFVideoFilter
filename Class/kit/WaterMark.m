//
//  WaterMark.m
//  FFMpeg
//
//  Created by 刘彦直 on 2019/3/11.
//  Copyright © 2019 刘彦直. All rights reserved.
//

#import "WaterMark.h"

@implementation WaterMark

- (instancetype)initWithPath: (NSString*)path rect: (CGRect)rect {
    self = [super init];
    if (self) {
        self.path = path;
        self.rect = rect;
    }
    return self;
}

@end
