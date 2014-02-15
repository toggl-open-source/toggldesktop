//
//  ErrorHandler.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "kopsik_api.h"
#import "UIEvents.h"

void handle_result(kopsik_api_result result, const char *err);
void handle_error(const char *err);