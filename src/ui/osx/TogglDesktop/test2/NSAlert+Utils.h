//
//  NSAlert+Utils.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/12/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSAlert (Utils)

+ (NSAlert *)alertWithMessageText:(NSString *)message informativeTextWithFormat:(NSString *)format, ...;

@end

NS_ASSUME_NONNULL_END
