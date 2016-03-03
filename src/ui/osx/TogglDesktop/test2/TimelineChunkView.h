//
//  TimelineChunkView.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 26/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "toggl_api.h"

@interface TimelineChunkView : NSObject
- (void)load:(TogglTimelineChunkView *)data;
@property uint64_t Started;
@property int activeDuration;
@property uint64_t EntryStart;
@property uint64_t EntryEnd;
@property uint64_t CalculatedHeight;
@property (strong) NSString *StartTimeString;
@property (strong) NSMutableArray *Events;
@property (strong) NSMutableArray *Entries;
@property (strong) NSString *EntryDescription;
@end
