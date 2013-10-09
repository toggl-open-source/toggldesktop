//
//  NSLocale+UnitTestSwizzling.m
//  ISO8601ForCocoa
//
//  Created by Matthias Bauch on 8/29/13.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import "NSLocale+UnitTestSwizzling.h"
#import "MBMockLocale.h"
#import <objc/runtime.h>
#import <objc/message.h>

@implementation NSLocale (TestSwizzling)

void SwizzleClassMethod(Class c, SEL orig, SEL new) {
    
    Method origMethod = class_getClassMethod(c, orig);
    Method newMethod = class_getClassMethod(c, new);
    
    c = object_getClass((id)c);
    
    if(class_addMethod(c, orig, method_getImplementation(newMethod), method_getTypeEncoding(newMethod)))
        class_replaceMethod(c, new, method_getImplementation(origMethod), method_getTypeEncoding(origMethod));
    else
        method_exchangeImplementations(origMethod, newMethod);
}

+ (NSLocale *)mockCurrentLocale {
    return (NSLocale *)[[MBMockLocale alloc] init];
}

@end
