//
//  ProxySettings.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 10/05/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ProxySettings : NSObject
@property BOOL use_proxy;
@property NSString *proxy_host;
@property long proxy_port;
@property NSString *proxy_username;
@property NSString *proxy_password;
@end
