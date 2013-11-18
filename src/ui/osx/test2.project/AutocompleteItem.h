//
//  AutocompleteItem.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface AutocompleteItem : NSObject
- (void)load:(KopsikAutocompleteItem *)data;
@property NSString *Text;
@property long ClientID;
@property long ProjectID;
@property long TaskID;
@property long TimeEntryID;
@end
