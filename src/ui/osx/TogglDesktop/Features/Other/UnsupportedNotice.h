//
//  UnsupportedNotice.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 23/11/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface UnsupportedNotice : NSAlert

@property (nonatomic, assign, readonly) BOOL unsupportedOS;

+ (instancetype)sharedInstance;

- (BOOL)validateOSVersion;
- (void)showNotice;

@end

NS_ASSUME_NONNULL_END
