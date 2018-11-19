//
//  AppIconFactory.h
//  TogglDesktop
//
//  Created by Nghia Tran on 11/19/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AppIconType) {
    AppIconTypeActive,
    AppIconTypeDefault,
};

@interface AppIconFactory : NSObject

+(NSImage *)appIconWithType:(AppIconType) type;

@end

NS_ASSUME_NONNULL_END
