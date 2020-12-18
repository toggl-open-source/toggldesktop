//
//  SPUDownloaderSession.h
//  Sparkle
//
//  Created by Deadpikle on 12/20/17.
//  Copyright © 2017 Sparkle Project. All rights reserved.
//

#if __has_feature(modules)
@import Foundation;
#else
#import <Foundation/Foundation.h>
#endif
#import <Sparkle/SPUDownloader.h>
#import <Sparkle/SPUDownloaderProtocol.h>

NS_CLASS_AVAILABLE(NSURLSESSION_AVAILABLE, 7_0)
@interface SPUDownloaderSession : SPUDownloader <SPUDownloaderProtocol>

@end
