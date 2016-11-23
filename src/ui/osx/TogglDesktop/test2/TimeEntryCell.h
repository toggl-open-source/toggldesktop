//
//  TimeEntryCell.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimeEntryViewItem.h"
#import "NSHoverButton.h"

@interface TimeEntryCell : NSTableCellView
@property (nonatomic, strong) IBOutlet NSTextField *descriptionTextField;
@property (nonatomic, strong) IBOutlet NSTextField *projectTextField;
@property (nonatomic, strong) IBOutlet NSImageView *billableFlag;
@property (nonatomic, strong) IBOutlet NSImageView *tagFlag;
@property (nonatomic, strong) IBOutlet NSTextField *durationTextField;
@property (strong) NSString *GUID;
@property (strong) NSString *GroupName;
@property BOOL Group;
@property BOOL GroupOpen;
@property long long GroupItemCount;
@property (strong) IBOutlet NSBox *durationBox;
@property (strong) IBOutlet NSBox *backgroundBox;
@property (strong) IBOutlet NSImageView *unsyncedIcon;
@property BOOL confrimless_delete;
@property (weak) IBOutlet NSBox *DescriptionProjectBox;
@property (weak) IBOutlet NSLayoutConstraint *descriptionBoxTrail;
@property (weak) IBOutlet NSLayoutConstraint *descriptionBoxLead;
@property (weak) IBOutlet NSHoverButton *groupToggleButton;
@property (weak) IBOutlet NSHoverButton *continueButton;
- (void)render:(TimeEntryViewItem *)view_item;
- (IBAction)continueTimeEntry:(id)sender;
- (IBAction)toggleGroup:(id)sender;
- (void)focusFieldName;
- (void)setFocused;
- (void)setupGroupMode;
- (void)openEdit;
@end

