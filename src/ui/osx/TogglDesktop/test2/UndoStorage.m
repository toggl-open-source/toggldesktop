//
//  UndoStorage.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/4/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "UndoStorage.h"

@implementation UndoPayload

-(instancetype) initWithName:(NSString *) name info:(NSString *) info {
    if (self = [super init]) {
        self.name = name;
        self.info = info;
    }
    return self;
}

@end

@interface UndoStorage ()

@property (strong, nonatomic) NSCache<NSString *, UndoPayload *> *cache;

@end

@implementation UndoStorage

+(instancetype) shared {
    static UndoStorage* instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[UndoStorage alloc] init];
    });
    return instance;
}

-(instancetype)init {
    self = [super init];
    if (self) {
        self.cache = [[NSCache<NSString *, UndoPayload *> alloc] init];
    }
    return self;
}

-(void) setPayload:(UndoPayload *) payload forGUID:(NSString *) guid {
    [self.cache setObject:payload forKey:guid];
}

-(UndoPayload *) getPayloadForGUID:(NSString *) guid {
    return [self.cache objectForKey:guid];
}

@end
