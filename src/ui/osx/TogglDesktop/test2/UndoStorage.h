//
//  UndoStorage.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/4/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface UndoPayload : NSObject

@property (copy, nonatomic) NSString *name;
@property (copy, nonatomic) NSString *info;

-(instancetype) initWithName:(NSString *) name info:(NSString *) info;

@end

@interface UndoStorage : NSObject
+(instancetype) shared;
-(void) setPayload:(UndoPayload *) payload forGUID:(NSString *) guid;
-(UndoPayload *) getPayloadForGUID:(NSString *) guid;

@end

NS_ASSUME_NONNULL_END
