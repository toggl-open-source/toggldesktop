//
//  MainWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "MainWindowController.h"
#import <Carbon/Carbon.h>
#import "LoginViewController.h"
#import "OverlayViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "DisplayCommand.h"
#import "TrackingService.h"
#import "TogglDesktop-Swift.h"
#import "TimelineDisplayCommand.h"
#import "TimerEditViewController.h"

@interface MainWindowController () <TouchBarServiceDelegate, NSWindowDelegate, InAppMessageViewControllerDelegate>
@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet NSView *mainView;
@property (nonatomic, strong) LoginViewController *loginViewController;
@property (nonatomic, strong) OverlayViewController *overlayViewController;
@property (nonatomic, strong) MainDashboardViewController *mainDashboardViewController;
@property (nonatomic, strong) SystemMessageView *messageView;
@property (nonatomic, strong) InAppMessageViewController *inappMessageView;
@property (nonatomic, assign) CGFloat troubleBoxDefaultHeight;
@end

@implementation MainWindowController

extern void *ctx;

- (id)initWithWindow:(NSWindow *)window
{
	self = [super initWithWindow:window];
	if (self)
	{
		self.loginViewController = [[LoginViewController alloc] initWithNibName:@"LoginViewController" bundle:nil];
		self.mainDashboardViewController = [[MainDashboardViewController alloc] initWithNibName:@"MainDashboardViewController" bundle:nil];
		self.overlayViewController = [[OverlayViewController alloc] initWithNibName:@"OverlayViewController" bundle:nil];

		[self.loginViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self.mainDashboardViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self.overlayViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayLogin:)
													 name:kDisplayLogin
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryList:)
													 name:kDisplayTimeEntryList
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayOverlay:)
													 name:kDisplayOverlay
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayError:)
													 name:kDisplayError
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(stopDisplayError:)
													 name:kHideDisplayError
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayOnlineState:)
													 name:kDisplayOnlineState
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeline:)
													 name:kDisplayTimeline
												   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(startDisplayInAppMessage:)
                                                     name:kStartDisplayInAppMessage
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(startOnboardingNotification:)
                                                     name:kStartDisplayOnboarding
                                                   object:nil];
	}
	return self;
}

- (void)windowDidLoad
{
	[super windowDidLoad];
    self.window.delegate = self;

	// Tracking the size of window after loaded
	[self trackWindowSize];

	// Error View
	[self initErrorView];
	[self setInitialWindowSizeIfNeed];

	// Touch bar
	[self initTouchBar];

    [self handleOnboarding];
}

- (void)initTouchBar
{
    if (@available(macOS 10.12.2, *)) {
        [TouchBarService shared].delegate = self;
    }
}

- (void)initErrorView {
	self.messageView = [SystemMessageView initFromXib];
	self.messageView.translatesAutoresizingMaskIntoConstraints = NO;
	[self.contentView addSubview:self.messageView];

	[self.messageView addConstraint:[NSLayoutConstraint constraintWithItem:self.messageView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:270.0]];
	NSLayoutConstraint *height = [NSLayoutConstraint constraintWithItem:self.messageView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:38.0];
	// Message View should be expandable depend on the length of text
	height.priority = NSLayoutPriorityDefaultLow;
	[self.messageView addConstraint:height];
	[self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:self.contentView attribute:NSLayoutAttributeTrailing relatedBy:NSLayoutRelationEqual toItem:self.messageView attribute:NSLayoutAttributeTrailing multiplier:1.0 constant:0]];
	[self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:self.contentView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.messageView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0]];
    [self.contentView addConstraint:[NSLayoutConstraint constraintWithItem:self.messageView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:self.contentView attribute:NSLayoutAttributeTop multiplier:1 constant:16]];

	// Hidden by default
	self.messageView.hidden = YES;

	// Register
	[self.messageView registerToSystemMessage];
}

- (void) initInAppMessageView
{
    self.inappMessageView = [[InAppMessageViewController alloc] initWithNibName:@"InAppMessageViewController" bundle:nil];
    self.inappMessageView.delegate = self;
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self displayLogin:notification.object];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		[self.contentView addSubview:self.loginViewController.view];
		[self.loginViewController.view setFrame:self.contentView.bounds];
		self.loginViewController.view.hidden = NO;

		[self.mainDashboardViewController.view removeFromSuperview];
		[self.overlayViewController.view removeFromSuperview];

		// Reset
        if (@available(macOS 10.12.2, *)) 
		{
            [[TouchBarService shared] resetContent];
        }

        // Reset the apple state
        #ifdef APP_STORE
        if (@available(macOS 10.15, *)) {
            [[AppleAuthenticationService shared] reset];
        }
        #endif
	}
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification
{
	[self displayTimeEntryList:notification.object];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		if ([self.loginViewController.view superview] != nil
			|| [self.mainDashboardViewController.view superview] == nil)
		{
			// Close error when loging in
			[self closeError];
			[self.loginViewController resetLoader];

			self.loginViewController.view.hidden = YES;
			[self.contentView addSubview:self.mainDashboardViewController.view];
			[self.mainDashboardViewController.view setFrame:self.contentView.bounds];

			[self.loginViewController.view removeFromSuperview];
			[self.overlayViewController.view removeFromSuperview];

			[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kFocusTimer
																		object:nil];

			// Prepare the Touch bar
            if (@available(macOS 10.12.2, *)) {
                [[TouchBarService shared] prepareContent];
            }
		}
	}
}

- (void)startDisplayTimeline:(NSNotification *)notification
{
	[self displayTimeline:notification.object];
}

- (void)displayTimeline:(TimelineDisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		[self.mainDashboardViewController timelineBtnOnTap:self];
	}
}

- (void)startDisplayOverlay:(NSNotification *)notification
{
	[self displayOverlay:notification.object];
}

- (void)displayOverlay:(NSNumber *)type
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	// Setup overlay content

	[self.overlayViewController setType:[type intValue]];

	[self.contentView addSubview:self.overlayViewController.view];
	[self.overlayViewController.view setFrame:self.contentView.bounds];

	[self.loginViewController.view removeFromSuperview];
	[self.mainDashboardViewController.view removeFromSuperview];
}

- (void)startDisplayError:(NSNotification *)notification
{
	[self displayError:notification.object];
}

- (void)displayError:(NSString *)msg
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSString *errorMessage = msg == nil ? @"Error" : msg;
	[[SystemMessage shared] presentError:errorMessage subTitle:nil];

	// Reset loader if there is error
	// Have to check if login is present
	if (self.loginViewController.view.superview != nil)
	{
		[self.loginViewController resetLoader];
	}
}

- (void)startDisplayOnlineState:(NSNotification *)notification
{
	[self displayOnlineState:notification.object];
}

- (void)displayOnlineState:(NSNumber *)status
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	switch ([status intValue])
	{
		case 1 :
			[[SystemMessage shared] presentOffline:@"Error" subTitle:@"Offline, no network"];
			break;
		case 2 :
			[[SystemMessage shared] presentOffline:@"Error" subTitle:@"Offline, Toggl not responding"];
			break;
		default :
			[self closeError];
			break;
	}

    // Have to check if login is present
    if (self.loginViewController.view.superview != nil)
    {
        [self.loginViewController resetLoader];
    }
}

- (void)stopDisplayError:(NSNotification *)notification
{
	[self closeError];
}

- (void)closeError
{
	self.messageView.hidden = YES;
}

- (void)keyUp:(NSEvent *)event
{
	if ([event keyCode] == kVK_DownArrow && ([event modifierFlags] & NSShiftKeyMask))
	{
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kFocusListing
																	object:nil];
	}
	else
	{
		[super keyUp:event];
	}
}

- (BOOL)isEditOpened
{
	return self.mainDashboardViewController.timeEntryController.isEditorOpen;
}

- (void)trackWindowSize
{
	if (self.window == nil)
	{
		return;
	}
	[[TrackingService sharedInstance] trackWindowSize:self.window.frame.size];
}

- (void)setWindowMode:(WindowMode)mode
{
	switch (mode)
	{
		case WindowModeAlwaysOnTop :
			[self.window setLevel:NSFloatingWindowLevel];
			self.window.collectionBehavior = NSWindowCollectionBehaviorManaged;
			break;
		case WindowModeDefault :
			[self.window setLevel:NSNormalWindowLevel];
			break;
	}
}

- (void)setInitialWindowSizeIfNeed
{
	if (self.contentView == nil || self.mainDashboardViewController.timerController == nil)
	{
		return;
	}

	if (self.contentView.frame.size.height - 2 <= self.mainDashboardViewController.timerController.view.frame.size.height)
	{
		[self.window setContentSize:CGSizeMake(400, 600)];
	}
}

#pragma mark - Touch Bar

- (void)touchBarServiceStartTimeEntryOnTap
{
	[self.mainDashboardViewController.timerController startButtonClicked:self];
}

#pragma mark - Timeline Menu

- (IBAction)zoomInBtnOnTap:(id)sender
{
    [self.mainDashboardViewController.timelineController zoomLevelIncreaseOnChange:sender];
}

- (IBAction)zoomOutBtnOnTap:(id)sender
{
    [self.mainDashboardViewController.timelineController zoomLevelDecreaseOnChange:sender];
}

- (IBAction)showTimeEntryTabBtnOnTap:(id)sender
{
    [self.mainDashboardViewController listBtnOnTap:sender];
}

- (IBAction)showTimelineTabBtnOnTap:(id)sender
{
    [self.mainDashboardViewController timelineBtnOnTap:sender];
}

- (IBAction)nextDayMenuOnClick:(id)sender
{
    [self.mainDashboardViewController nextDay];
}

- (IBAction)previouosDayMenuOnClick:(id)sender
{
    [self.mainDashboardViewController previousDay];
}

#pragma mark - In app message

- (void)startDisplayInAppMessage:(NSNotification *)notification
{
    if (![notification.object isKindOfClass:[InAppMessage class]]) {
        return;
    }

    if (!self.inappMessageView)
    {
        [self initInAppMessageView];
    }
    self.inappMessageView.view.hidden = YES;
    [self.contentView addSubview:self.inappMessageView.view];
    [self.inappMessageView.view edgesToSuperView];

    // Update UI
    InAppMessage *message = (InAppMessage *) notification.object;
    [self.inappMessageView update:message];

    // Prepare for animation
    [self.inappMessageView prepareForAnimation];
    self.inappMessageView.view.hidden = NO;
    [self.inappMessageView present];
}

- (void)InAppMessageViewControllerShouldDismiss
{
    [self.inappMessageView.view removeFromSuperview];
}

- (NSTouchBar *)makeTouchBar
{
    return [[TouchBarService shared] makeTouchBar];
}

#pragma mark - Onboarding

- (void)startOnboardingNotification:(NSNotification *) noti {
    NSNumber *onboardingTypeNumber = (NSNumber *) noti.object;
    OnboardingHint hint = [OnboardingServiceObjc hintFromValue:onboardingTypeNumber.integerValue];

    if (hint == OnboardingHintNone) {
        return;
    }
    NSView *presentView = [self getOnboardingViewWithHint:hint];
    if (!presentView) {
        return;
    }
    [OnboardingServiceObjc presentWithHintValue:onboardingTypeNumber.integerValue atView:presentView];
}

-(void) handleOnboarding {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [OnboardingServiceObjc presentWithHintValue:0 atView:self.mainDashboardViewController.timelineBtn];
    });
}

- (NSView * __nullable) getOnboardingViewWithHint:(OnboardingHint) hint
{
    switch (hint) {
        case OnboardingHintNewUser:
            return (NSView *) self.mainDashboardViewController.timerController.mainBox;
        case OnboardingHintOldUser:
            return (NSView *) self.mainDashboardViewController.timerController.mainBox;
        case OnboardingHintManualMode:
            return (NSView *) self.mainDashboardViewController.timerController.mainBox;
        case OnboardingHintTimelineTab:
            return self.mainDashboardViewController.timelineBtn;
        case OnboardingHintEditTimeEntry:
            return [self.mainDashboardViewController.timeEntryController firstTimeEntryCellForOnboarding];
        case OnboardingHintTimelineTimeEntry:
            return self.mainDashboardViewController.timelineController.collectionView; // Need update
        case OnboardingHintTimelineView:
            return self.mainDashboardViewController.timelineController.collectionView;
        case OnboardingHintTimelineActivity:
            return self.mainDashboardViewController.timelineController.collectionView;
        case OnboardingHintRecordActivity:
            return self.mainDashboardViewController.timelineController.collectionView;
        case OnboardingHintNone:
            return nil;
        default:
            return nil;
    }
}
@end
