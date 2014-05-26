//
//  AutocompleteItem.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface AutocompleteItem : NSObject
+ (NSMutableArray *)loadAll:(KopsikAutocompleteItem *)first;
- (void)load:(KopsikAutocompleteItem *)data;
- (void)save:(KopsikAutocompleteItem *)data;
@property NSString *Text;
@property NSString *Description;
@property NSString *ProjectAndTaskLabel;
@property NSString *ProjectColor;
@property uint64_t ProjectID;
@property uint64_t TaskID;
@property uint64_t Type;
@end
