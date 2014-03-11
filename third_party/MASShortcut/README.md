# Intro

Some time ago Cocoa developers used a brilliant framework [ShortcutRecorder](http://wafflesoftware.net/shortcut/) for managing keyboard shortcuts in application preferences. However, it became incompatible with a new plugin architecture of Xcode 4.

The project MASShortcut introduces modern API and user interface for recording, storing and using global keyboard shortcuts. All code is compatible with Xcode 4.3, Mac OS X 10.7 and the sandboxed environment.

# Usage

I hope, it is really easy:

```objective-c
// Drop a custom view into XIB, set its class to MASShortcutView and its height to 19. If you select another appearance style look up the correct values in MASShortcutView.h
@property (nonatomic, weak) IBOutlet MASShortcutView *shortcutView;
	
// Think up a preference key to store a global shortcut between launches
NSString *const kPreferenceGlobalShortcut = @"GlobalShortcut";

// Assign the preference key and the shortcut view will take care of persistence
self.shortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcut;

// Execute your block of code automatically when user triggers a shortcut from preferences
[MASShortcut registerGlobalShortcutWithUserDefaultsKey:kPreferenceGlobalShortcut handler:^{
    // Let me know if you find a better or more convenient API.
}];
```

To set an example, I made a  demo project: [MASShortcutDemo](https://github.com/shpakovski/MASShortcutDemo). Enjoy!

#Notifications
By registering for KVO notifications from `NSUserDefaultsController`, you can get a callback whenever a user changes the shortcut, allowing you to perform any UI updates, or other code handling tasks.

This is just as easy to implement:
    
```objective-c
// Declare an ivar for key path in the user defaults controller
NSString *_observableKeyPath;
    
// Make a global context reference
void *kGlobalShortcutContext = &kGlobalShortcutContext;
    
// Implement when loading view
_observableKeyPath = [@"values." stringByAppendingString:kPreferenceGlobalShortcut];
[[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:_observableKeyPath
                                                             options:NSKeyValueObservingOptionInitial
                                                             context:kGlobalShortcutContext];

// Capture the KVO change and do something
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)obj
                        change:(NSDictionary *)change context:(void *)ctx
{
    if (ctx == kGlobalShortcutContext) {
        NSLog(@"Shortcut has changed");
    }
    else {
        [super observeValueForKeyPath:keyPath ofObject:obj change:change context:ctx];
    }
}

// Do not forget to remove the observer
[[NSUserDefaultsController sharedUserDefaultsController] removeObserver:self
                                                             forKeyPath:_observableKeyPath
                                                                context:kGlobalShortcutContext];
```

# Non-ARC Version

If you like retain/release, please check out these forks: [heardrwt/MASShortcut](https://github.com/heardrwt/MASShortcut) and [chendo/MASShortcut](https://github.com/chendo/MASShortcut). However, the preferred way is to enable the `-fobjc-arc` in Xcode source options.

# Copyright

MASShortcut is licensed under the 2-clause BSD license.
