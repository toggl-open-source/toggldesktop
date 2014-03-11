#import "MASShortcut.h"

NSString *const MASShortcutKeyCode = @"KeyCode";
NSString *const MASShortcutModifierFlags = @"ModifierFlags";

@implementation MASShortcut {
    NSUInteger _keyCode; // NSNotFound if empty
    NSUInteger _modifierFlags; // 0 if empty
}

@synthesize modifierFlags = _modifierFlags;
@synthesize keyCode = _keyCode;

#pragma mark -

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInteger:(self.keyCode != NSNotFound ? (NSInteger)self.keyCode : - 1) forKey:MASShortcutKeyCode];
    [coder encodeInteger:(NSInteger)self.modifierFlags forKey:MASShortcutModifierFlags];
}

- (id)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self) {
        NSInteger code = [decoder decodeIntegerForKey:MASShortcutKeyCode];
        self.keyCode = (code < 0 ? NSNotFound : (NSUInteger)code);
        self.modifierFlags = [decoder decodeIntegerForKey:MASShortcutModifierFlags];
    }
    return self;
}

- (id)initWithKeyCode:(NSUInteger)code modifierFlags:(NSUInteger)flags
{
    self = [super init];
    if (self) {
        _keyCode = code;
        _modifierFlags = MASShortcutClear(flags);
    }
    return self;
}

+ (MASShortcut *)shortcutWithKeyCode:(NSUInteger)code modifierFlags:(NSUInteger)flags
{
    return [[self alloc] initWithKeyCode:code modifierFlags:flags];
}

+ (MASShortcut *)shortcutWithEvent:(NSEvent *)event
{
    return [[self alloc] initWithKeyCode:event.keyCode modifierFlags:event.modifierFlags];
}

+ (MASShortcut *)shortcutWithData:(NSData *)data
{
    id shortcut = (data ? [NSKeyedUnarchiver unarchiveObjectWithData:data] : nil);
    return shortcut;
}

#pragma mark - Shortcut accessors

- (NSData *)data
{
    return [NSKeyedArchiver archivedDataWithRootObject:self];
}

- (void)setModifierFlags:(NSUInteger)value
{
    _modifierFlags = MASShortcutClear(value);
}

- (UInt32)carbonKeyCode
{
    return (self.keyCode == NSNotFound ? 0 : (UInt32)self.keyCode);
}

- (UInt32)carbonFlags
{
    return MASShortcutCarbonFlags(self.modifierFlags);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@%@", self.modifierFlagsString, self.keyCodeString];
}

- (NSString *)keyCodeStringForKeyEquivalent
{
    NSString *keyCodeString = self.keyCodeString;
    if (keyCodeString.length > 1) {
        switch (self.keyCode) {
            case kVK_F1: return MASShortcutChar(0xF704);
            case kVK_F2: return MASShortcutChar(0xF705);
            case kVK_F3: return MASShortcutChar(0xF706);
            case kVK_F4: return MASShortcutChar(0xF707);
            case kVK_F5: return MASShortcutChar(0xF708);
            case kVK_F6: return MASShortcutChar(0xF709);
            case kVK_F7: return MASShortcutChar(0xF70a);
            case kVK_F8: return MASShortcutChar(0xF70b);
            case kVK_F9: return MASShortcutChar(0xF70c);
            case kVK_F10: return MASShortcutChar(0xF70d);
            case kVK_F11: return MASShortcutChar(0xF70e);
            case kVK_F12: return MASShortcutChar(0xF70f);
            // From this point down I am guessing F13 etc come sequentially, I don't have a keyboard to test.
            case kVK_F13: return MASShortcutChar(0xF710);
            case kVK_F14: return MASShortcutChar(0xF711);
            case kVK_F15: return MASShortcutChar(0xF712);
            case kVK_F16: return MASShortcutChar(0xF713);
            case kVK_F17: return MASShortcutChar(0xF714);
            case kVK_F18: return MASShortcutChar(0xF715);
            case kVK_F19: return MASShortcutChar(0xF716);
            case kVK_Space: return MASShortcutChar(0x20);
            default: return @"";
        }
    }
    return keyCodeString.lowercaseString;
}

- (NSString *)keyCodeString
{
    // Some key codes don't have an equivalent
    switch (self.keyCode) {
        case NSNotFound: return @"";
        case kVK_F1: return @"F1";
        case kVK_F2: return @"F2";
        case kVK_F3: return @"F3";
        case kVK_F4: return @"F4";
        case kVK_F5: return @"F5";
        case kVK_F6: return @"F6";
        case kVK_F7: return @"F7";
        case kVK_F8: return @"F8";
        case kVK_F9: return @"F9";
        case kVK_F10: return @"F10";
        case kVK_F11: return @"F11";
        case kVK_F12: return @"F12";
        case kVK_F13: return @"F13";
        case kVK_F14: return @"F14";
        case kVK_F15: return @"F15";
        case kVK_F16: return @"F16";
        case kVK_F17: return @"F17";
        case kVK_F18: return @"F18";
        case kVK_F19: return @"F19";
        case kVK_Space: return NSLocalizedString(@"Space", @"Shortcut glyph name for SPACE key");
        case kVK_Escape: return MASShortcutChar(kMASShortcutGlyphEscape);
        case kVK_Delete: return MASShortcutChar(kMASShortcutGlyphDeleteLeft);
        case kVK_ForwardDelete: return MASShortcutChar(kMASShortcutGlyphDeleteRight);
        case kVK_LeftArrow: return MASShortcutChar(kMASShortcutGlyphLeftArrow);
        case kVK_RightArrow: return MASShortcutChar(kMASShortcutGlyphRightArrow);
        case kVK_UpArrow: return MASShortcutChar(kMASShortcutGlyphUpArrow);
        case kVK_DownArrow: return MASShortcutChar(kMASShortcutGlyphDownArrow);
        case kVK_Help: return MASShortcutChar(kMASShortcutGlyphHelp);
        case kVK_PageUp: return MASShortcutChar(kMASShortcutGlyphPageUp);
        case kVK_PageDown: return MASShortcutChar(kMASShortcutGlyphPageDown);
        case kVK_Tab: return MASShortcutChar(kMASShortcutGlyphTabRight);
        case kVK_Return: return MASShortcutChar(kMASShortcutGlyphReturnR2L);
            
        // Keypad
        case kVK_ANSI_Keypad0: return @"0";
        case kVK_ANSI_Keypad1: return @"1";
        case kVK_ANSI_Keypad2: return @"2";
        case kVK_ANSI_Keypad3: return @"3";
        case kVK_ANSI_Keypad4: return @"4";
        case kVK_ANSI_Keypad5: return @"5";
        case kVK_ANSI_Keypad6: return @"6";
        case kVK_ANSI_Keypad7: return @"7";
        case kVK_ANSI_Keypad8: return @"8";
        case kVK_ANSI_Keypad9: return @"9";
        case kVK_ANSI_KeypadDecimal: return @".";
        case kVK_ANSI_KeypadMultiply: return @"*";
        case kVK_ANSI_KeypadPlus: return @"+";
        case kVK_ANSI_KeypadClear: return MASShortcutChar(kMASShortcutGlyphPadClear);
        case kVK_ANSI_KeypadDivide: return @"/";
        case kVK_ANSI_KeypadEnter: return MASShortcutChar(kMASShortcutGlyphReturn);
        case kVK_ANSI_KeypadMinus: return @"–";
        case kVK_ANSI_KeypadEquals: return @"=";
            
        // Hardcode
        case 119: return MASShortcutChar(kMASShortcutGlyphSoutheastArrow);
        case 115: return MASShortcutChar(kMASShortcutGlyphNorthwestArrow);
    }
    
    // Everything else should be printable so look it up in the current keyboard
    OSStatus error = noErr;
    NSString *keystroke = nil;
    TISInputSourceRef inputSource = TISCopyCurrentKeyboardLayoutInputSource();
    if (inputSource) {
        CFDataRef layoutDataRef = TISGetInputSourceProperty(inputSource, kTISPropertyUnicodeKeyLayoutData);
        if (layoutDataRef) {
            UCKeyboardLayout *layoutData = (UCKeyboardLayout *)CFDataGetBytePtr(layoutDataRef);
            UniCharCount length = 0;
            UniChar  chars[256] = { 0 };
            UInt32 deadKeyState = 0;
            error = UCKeyTranslate(layoutData, (UInt16)self.keyCode, kUCKeyActionDisplay, 0, // No modifiers
                                   LMGetKbdType(), kUCKeyTranslateNoDeadKeysMask, &deadKeyState,
                                   sizeof(chars) / sizeof(UniChar), &length, chars);
            keystroke = ((error == noErr) && length ? [NSString stringWithCharacters:chars length:length] : @"");
        }
        CFRelease(inputSource);
    }
    
    // Validate keystroke
    if (keystroke.length) {
        static NSMutableCharacterSet *validChars = nil;
        if (validChars == nil) {
            validChars = [[NSMutableCharacterSet alloc] init];
            [validChars formUnionWithCharacterSet:[NSCharacterSet alphanumericCharacterSet]];
            [validChars formUnionWithCharacterSet:[NSCharacterSet punctuationCharacterSet]];
            [validChars formUnionWithCharacterSet:[NSCharacterSet symbolCharacterSet]];
        }
        for (NSUInteger i = 0, length = keystroke.length; i < length; i++) {
            if (![validChars characterIsMember:[keystroke characterAtIndex:i]]) {
                keystroke = @"";
                break;
            }
        }
    }
    
    // Finally, we've got a shortcut!
    return keystroke.uppercaseString;
}

- (NSString *)modifierFlagsString
{
    unichar chars[4];
    NSUInteger count = 0;
    // These are in the same order as the menu manager shows them
    if (self.modifierFlags & NSControlKeyMask) chars[count++] = kControlUnicode;
    if (self.modifierFlags & NSAlternateKeyMask) chars[count++] = kOptionUnicode;
    if (self.modifierFlags & NSShiftKeyMask) chars[count++] = kShiftUnicode;
    if (self.modifierFlags & NSCommandKeyMask) chars[count++] = kCommandUnicode;
    return (count ? [NSString stringWithCharacters:chars length:count] : @"");
}

#pragma mark - Validation logic

- (BOOL)shouldBypass
{
    NSString *codeString = self.keyCodeString;
    return (self.modifierFlags == NSCommandKeyMask) && ([codeString isEqualToString:@"W"] || [codeString isEqualToString:@"Q"]);
}

BOOL MASShortcutAllowsAnyHotkeyWithOptionModifier = NO;

+ (void)setAllowsAnyHotkeyWithOptionModifier:(BOOL)allow
{
    MASShortcutAllowsAnyHotkeyWithOptionModifier = allow;
}

+ (BOOL)allowsAnyHotkeyWithOptionModifier
{
    return MASShortcutAllowsAnyHotkeyWithOptionModifier;
}

- (BOOL)isValid
{
    // Allow any function key with any combination of modifiers
    BOOL includesFunctionKey = ((_keyCode == kVK_F1) || (_keyCode == kVK_F2) || (_keyCode == kVK_F3) || (_keyCode == kVK_F4) ||
                                (_keyCode == kVK_F5) || (_keyCode == kVK_F6) || (_keyCode == kVK_F7) || (_keyCode == kVK_F8) ||
                                (_keyCode == kVK_F9) || (_keyCode == kVK_F10) || (_keyCode == kVK_F11) || (_keyCode == kVK_F12) ||
                                (_keyCode == kVK_F13) || (_keyCode == kVK_F14) || (_keyCode == kVK_F15) || (_keyCode == kVK_F16) ||
                                (_keyCode == kVK_F17) || (_keyCode == kVK_F18) || (_keyCode == kVK_F19) || (_keyCode == kVK_F20));
    if (includesFunctionKey) return YES;

    // Do not allow any other key without modifiers
    BOOL hasModifierFlags = (_modifierFlags > 0);
    if (!hasModifierFlags) return NO;

    // Allow any hotkey containing Control or Command modifier
    BOOL includesCommand = ((_modifierFlags & NSCommandKeyMask) > 0);
    BOOL includesControl = ((_modifierFlags & NSControlKeyMask) > 0);
    if (includesCommand || includesControl) return YES;

    // Allow Option key only in selected cases
    BOOL includesOption = ((_modifierFlags & NSAlternateKeyMask) > 0);
    if (includesOption) {

        // Always allow Option-Space and Option-Escape because they do not have any bind system commands
        if ((_keyCode == kVK_Space) || (_keyCode == kVK_Escape)) return YES;

        // Allow Option modifier with any key even if it will break the system binding
        if ([[self class] allowsAnyHotkeyWithOptionModifier]) return YES;
    }

    // The hotkey does not have any modifiers or violates system bindings
    return NO;
}

- (BOOL)isKeyEquivalent:(NSString *)keyEquivalent flags:(NSUInteger)flags takenInMenu:(NSMenu *)menu error:(NSError **)outError
{
    for (NSMenuItem *menuItem in menu.itemArray) {
        if (menuItem.hasSubmenu && [self isKeyEquivalent:keyEquivalent flags:flags takenInMenu:menuItem.submenu error:outError]) return YES;
        
        BOOL equalFlags = (MASShortcutClear(menuItem.keyEquivalentModifierMask) == flags);
        BOOL equalHotkeyLowercase = [menuItem.keyEquivalent.lowercaseString isEqualToString:keyEquivalent];
        
        // Check if the cases are different, we know ours is lower and that shift is included in our modifiers
        // If theirs is capitol, we need to add shift to their modifiers
        if (equalHotkeyLowercase && ![menuItem.keyEquivalent isEqualToString:keyEquivalent]) {
            equalFlags = (MASShortcutClear(menuItem.keyEquivalentModifierMask | NSShiftKeyMask) == flags);
        }
        
        if (equalFlags && equalHotkeyLowercase) {
            if (outError) {
                NSString *format = NSLocalizedString(@"This shortcut cannot be used because it is already used by the menu item ‘%@’.",
                                                     @"Message for alert when shortcut is already used");
                NSDictionary *info = [NSDictionary dictionaryWithObject:[NSString stringWithFormat:format, menuItem.title]
                                                                 forKey:NSLocalizedDescriptionKey];
                *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:0 userInfo:info];
            }
            return YES;
        }
    }
    return NO;
}

- (BOOL)isTakenError:(NSError **)outError
{
	CFArrayRef globalHotKeys;
	if (CopySymbolicHotKeys(&globalHotKeys) == noErr) {

        // Enumerate all global hotkeys and check if any of them matches current shortcut
        for (CFIndex i = 0, count = CFArrayGetCount(globalHotKeys); i < count; i++) {
            CFDictionaryRef hotKeyInfo = CFArrayGetValueAtIndex(globalHotKeys, i);
            CFNumberRef code = CFDictionaryGetValue(hotKeyInfo, kHISymbolicHotKeyCode);
            CFNumberRef flags = CFDictionaryGetValue(hotKeyInfo, kHISymbolicHotKeyModifiers);

            if (([(__bridge NSNumber *)code unsignedIntegerValue] == self.keyCode) &&
                ([(__bridge NSNumber *)flags unsignedIntegerValue] == self.carbonFlags)) {

                if (outError) {
                    NSString *description = NSLocalizedString(@"This combination cannot be used because it is already used by a system-wide "
                                                              @"keyboard shortcut.\nIf you really want to use this key combination, most shortcuts "
                                                              @"can be changed in the Keyboard & Mouse panel in System Preferences.",
                                                              @"Message for alert when shortcut is already used by the system");
                    NSDictionary *info = [NSDictionary dictionaryWithObject:description forKey:NSLocalizedDescriptionKey];
                    *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:0 userInfo:info];
                }
                return YES;
            }
        }
        CFRelease(globalHotKeys);
    }
    return [self isKeyEquivalent:self.keyCodeStringForKeyEquivalent flags:self.modifierFlags takenInMenu:[NSApp mainMenu] error:outError];
}

@end
