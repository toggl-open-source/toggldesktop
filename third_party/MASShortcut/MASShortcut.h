#import <Carbon/Carbon.h>

#define MASShortcutChar(char) [NSString stringWithFormat:@"%C", (unsigned short)(char)]
#define MASShortcutClear(flags) (flags & (NSControlKeyMask | NSShiftKeyMask | NSAlternateKeyMask | NSCommandKeyMask))
#define MASShortcutCarbonFlags(cocoaFlags) (\
    (cocoaFlags & NSCommandKeyMask ? cmdKey : 0) | \
    (cocoaFlags & NSAlternateKeyMask ? optionKey : 0) | \
    (cocoaFlags & NSControlKeyMask ? controlKey : 0) | \
    (cocoaFlags & NSShiftKeyMask ? shiftKey : 0))

// These glyphs are missed in Carbon.h
enum {
    kMASShortcutGlyphEject = 0x23CF,
    kMASShortcutGlyphClear = 0x2715,
	kMASShortcutGlyphDeleteLeft = 0x232B,
	kMASShortcutGlyphDeleteRight = 0x2326,
    kMASShortcutGlyphLeftArrow = 0x2190,
	kMASShortcutGlyphRightArrow = 0x2192,
	kMASShortcutGlyphUpArrow = 0x2191,
	kMASShortcutGlyphDownArrow = 0x2193,
	kMASShortcutGlyphEscape = 0x238B,
	kMASShortcutGlyphHelp = 0x003F,
    kMASShortcutGlyphPageDown = 0x21DF,
	kMASShortcutGlyphPageUp = 0x21DE,
	kMASShortcutGlyphTabRight = 0x21E5,
	kMASShortcutGlyphReturn = 0x2305,
	kMASShortcutGlyphReturnR2L = 0x21A9,	
	kMASShortcutGlyphPadClear = 0x2327,
	kMASShortcutGlyphNorthwestArrow = 0x2196,
	kMASShortcutGlyphSoutheastArrow = 0x2198,
} MASShortcutGlyph;

@interface MASShortcut : NSObject <NSCoding>

@property (nonatomic) NSUInteger keyCode;
@property (nonatomic) NSUInteger modifierFlags;
@property (nonatomic, readonly) UInt32 carbonKeyCode;
@property (nonatomic, readonly) UInt32 carbonFlags;
@property (nonatomic, readonly) NSString *keyCodeString;
@property (nonatomic, readonly) NSString *keyCodeStringForKeyEquivalent;
@property (nonatomic, readonly) NSString *modifierFlagsString;
@property (nonatomic, readonly) NSData *data;
@property (nonatomic, readonly) BOOL shouldBypass;
@property (nonatomic, readonly, getter = isValid) BOOL valid;

- (id)initWithKeyCode:(NSUInteger)code modifierFlags:(NSUInteger)flags;

+ (MASShortcut *)shortcutWithKeyCode:(NSUInteger)code modifierFlags:(NSUInteger)flags;
+ (MASShortcut *)shortcutWithEvent:(NSEvent *)anEvent;
+ (MASShortcut *)shortcutWithData:(NSData *)aData;

- (BOOL)isTakenError:(NSError **)error;

// The following API enable hotkeys with the Option key as the only modifier
// For example, Option-G will not generate © and Option-R will not paste ®
+ (void)setAllowsAnyHotkeyWithOptionModifier:(BOOL)allow;
+ (BOOL)allowsAnyHotkeyWithOptionModifier;

@end
