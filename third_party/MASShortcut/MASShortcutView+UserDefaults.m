#import "MASShortcutView+UserDefaults.h"
#import "MASShortcut.h"
#import <objc/runtime.h>

@interface MASShortcutDefaultsObserver : NSObject

@property (nonatomic, readonly) NSString *userDefaultsKey;
@property (nonatomic, readonly, weak) MASShortcutView *shortcutView;

- (id)initWithShortcutView:(MASShortcutView *)shortcutView userDefaultsKey:(NSString *)userDefaultsKey;

@end

#pragma mark -

@implementation MASShortcutView (UserDefaults)

void *MASAssociatedDefaultsObserver = &MASAssociatedDefaultsObserver;

- (NSString *)associatedUserDefaultsKey
{
    MASShortcutDefaultsObserver *defaultsObserver = objc_getAssociatedObject(self, MASAssociatedDefaultsObserver);
    return defaultsObserver.userDefaultsKey;
}

- (void)setAssociatedUserDefaultsKey:(NSString *)associatedUserDefaultsKey
{
    // First, stop observing previous shortcut view
    objc_setAssociatedObject(self, MASAssociatedDefaultsObserver, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    if (associatedUserDefaultsKey.length == 0) return;

    // Next, start observing current shortcut view
    MASShortcutDefaultsObserver *defaultsObserver = [[MASShortcutDefaultsObserver alloc] initWithShortcutView:self userDefaultsKey:associatedUserDefaultsKey];
    objc_setAssociatedObject(self, MASAssociatedDefaultsObserver, defaultsObserver, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

@end

#pragma mark -

@implementation MASShortcutDefaultsObserver {
    MASShortcut *_originalShortcut;
    BOOL _internalPreferenceChange;
    BOOL _internalShortcutChange;
}

@synthesize userDefaultsKey = _userDefaultsKey;
@synthesize shortcutView = _shortcutView;

#pragma mark -

- (id)initWithShortcutView:(MASShortcutView *)shortcutView userDefaultsKey:(NSString *)userDefaultsKey
{
    self = [super init];
    if (self) {
        _originalShortcut = shortcutView.shortcutValue;
        _shortcutView = shortcutView;
        _userDefaultsKey = userDefaultsKey.copy;
        [self startObservingShortcutView];
    }
    return self;
}

- (void)dealloc
{
    // __weak _shortcutView is not yet deallocated because it refers MASShortcutDefaultsObserver
    [self stopObservingShortcutView];
}

#pragma mark -

void *kShortcutValueObserver = &kShortcutValueObserver;

- (void)startObservingShortcutView
{
    // Read initial shortcut value from user preferences
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSData *data = [defaults dataForKey:_userDefaultsKey];
    _shortcutView.shortcutValue = [MASShortcut shortcutWithData:data];

    // Observe user preferences to update shortcut value when it changed
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(userDefaultsDidChange:) name:NSUserDefaultsDidChangeNotification object:defaults];

    // Observe the keyboard shortcut that user inputs by hand
    [_shortcutView addObserver:self forKeyPath:@"shortcutValue" options:0 context:kShortcutValueObserver];
}

- (void)userDefaultsDidChange:(NSNotification *)note
{
    // Ignore notifications posted from -[self observeValueForKeyPath:]
    if (_internalPreferenceChange) return;

    _internalShortcutChange = YES;
    NSData *data = [note.object dataForKey:_userDefaultsKey];
    _shortcutView.shortcutValue = [MASShortcut shortcutWithData:data];
    _internalShortcutChange = NO;
}

- (void)stopObservingShortcutView
{
    // Stop observing keyboard hotkeys entered by user in the shortcut view
    [_shortcutView removeObserver:self forKeyPath:@"shortcutValue" context:kShortcutValueObserver];

    // Stop observing user preferences
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSUserDefaultsDidChangeNotification object:[NSUserDefaults standardUserDefaults]];

    // Restore original hotkey in the shortcut view
    _shortcutView.shortcutValue = _originalShortcut;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == kShortcutValueObserver) {
        if (_internalShortcutChange) return;
        MASShortcut *shortcut = [object valueForKey:keyPath];
        _internalPreferenceChange = YES;

        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:(shortcut.data ?: [NSKeyedArchiver archivedDataWithRootObject:nil]) forKey:_userDefaultsKey];
        [defaults synchronize];

        _internalPreferenceChange = NO;
    }
    else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

@end
