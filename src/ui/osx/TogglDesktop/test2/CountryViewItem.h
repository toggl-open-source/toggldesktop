//
//  CountryViewItem.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 16/05/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@interface CountryViewItem : NSObject
+ (NSMutableArray *)loadAll:(TogglCountryView *)first;
- (void)load:(TogglCountryView *)data;
@property uint64_t ID;
@property BOOL VatApplicable;
@property (strong) NSString *Text;
@property (strong) NSString *Name;
@property (strong) NSString *VatPercentage;
@property (strong) NSString *VatRegex;
@property (strong) NSString *Code;
@end
