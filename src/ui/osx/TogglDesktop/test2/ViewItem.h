//
//  ViewItem.h
//  Toggl Desktop on the Mac
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ViewItem : NSObject
@property (assign, nonatomic) uint64_t ID;
@property (assign, nonatomic) uint64_t WID;
@property (copy, nonatomic) NSString *GUID;
@property (copy, nonatomic) NSString *Name;
@property (copy, nonatomic) NSString *workspaceName;
@property (assign, nonatomic) BOOL Premium;

+ (NSArray<ViewItem *> *)loadAll:(TogglGenericView *)first;
- (void)load:(TogglGenericView *)data;
@end


