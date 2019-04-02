//
//  DesktopLibraryBridge.h
//  TogglDesktop
//
//  Created by Nghia Tran on 4/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface DesktopLibraryBridge : NSObject

+ (instancetype)shared;

- (NSString *)createClientWithWorkspaceID:(uint64_t)workspaceID clientName:(NSString *)clientName;

@end

NS_ASSUME_NONNULL_END
