//
//  ErrorHandler.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "ErrorHandler.h"

void handle_error(kopsik_api_result result, const char *err) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
  }
}
