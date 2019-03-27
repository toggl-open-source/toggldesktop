//
//  TimelineEventView.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

NS_ASSUME_NONNULL_BEGIN

@interface TimelineEventView : NSObject
- (void)load:(TogglTimelineEventView *)data;
- (NSMutableAttributedString *)descriptionString;
- (void)updateSorter;
- (NSString *)prettyDuration;
@property (strong) NSString *Title;
@property (strong) NSString *Filename;
@property (strong) NSString *Sorter;
@property int Duration;
@property BOOL Header;
@end

NS_ASSUME_NONNULL_END
