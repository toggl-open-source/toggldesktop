//
//  MBMockLocale.m
//  ISO8601ForCocoa
//
//  Created by Matthias Bauch on 8/29/13.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import "MBMockLocale.h"

@implementation MBMockLocale {
    NSLocale *locale;
}

- (id)init {
    self = [super init];
    locale = [[NSLocale alloc] initWithLocaleIdentifier:@"de_DE"];
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    return [[[self class] allocWithZone:zone] init];
}

- (NSString *)localeIdentifier {
    return [locale localeIdentifier];
}

- (id)objectForKey:(NSString *)key {
    id object = [locale objectForKey:key];
    return object;
}

- (NSDictionary *)_prefs {
    NSMutableDictionary *prefs = [NSMutableDictionary dictionaryWithDictionary:[locale performSelector:@selector(_prefs)]];
    [prefs setObject:@YES forKey:@"AppleICUForce12HourTime"];
    return prefs;
}

@end
