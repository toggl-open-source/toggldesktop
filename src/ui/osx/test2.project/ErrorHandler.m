//
//  ErrorHandler.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "ErrorHandler.h"

void handle_result(_Bool result, const char *errmsg) {
  if (result) {
    return;
  }
  handle_error(errmsg);
}

void handle_error(const char *errmsg) {
  if (!errmsg ) {
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                      object:[NSString stringWithUTF8String:errmsg]];
}
