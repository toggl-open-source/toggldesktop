//
//  TrackingService.h
//  TogglDesktop
//
//  Created by Nghia Tran on 11/22/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface TrackingService : NSObject

+(instancetype) sharedInstance;

-(void)trackWindowSize:(NSSize) size;

@end

NS_ASSUME_NONNULL_END
