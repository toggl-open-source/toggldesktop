//
//  TimelineChunkView.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

NS_ASSUME_NONNULL_BEGIN

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

NS_ASSUME_NONNULL_END
