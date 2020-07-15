//
//  CountryViewItem.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 16/05/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CountryViewItem : NSObject
@property (assign, nonatomic) uint64_t ID;
@property (assign, nonatomic) BOOL VatApplicable;
@property (copy, nonatomic) NSString *Text;
@property (copy, nonatomic) NSString *Name;
@property (copy, nonatomic) NSString *VatPercentage;
@property (copy, nonatomic) NSString *VatRegex;
@property (copy, nonatomic) NSString *Code;

+ (NSMutableArray *)loadAll:(TogglCountryView *)first;
- (void)load:(TogglCountryView *)data;

@end
