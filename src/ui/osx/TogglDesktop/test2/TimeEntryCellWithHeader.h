//
//  TimeEntryCellWithHeader.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryCell.h"

@interface TimeEntryCellWithHeader : TimeEntryCell
@property (nonatomic, strong) IBOutlet NSTextField *formattedDateTextField;
@property (nonatomic, strong) IBOutlet NSTextField *dateDurationTextField;
@end

