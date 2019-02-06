//
//  MainWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "MainWindowController.h"
#import "LoginViewController.h"
#import "TimeEntryListViewController.h"
#import "OverlayViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "MenuItemTags.h"
#import "DisplayCommand.h"
#include <Carbon/Carbon.h>
#import "TrackingService.h"
#import "TogglDesktop-Swift.h"

@interface MainWindowController () <FloatingErrorViewDelegate>
@property (nonatomic, strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic, strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property (nonatomic, strong) IBOutlet OverlayViewController *overlayViewController;
@property (nonatomic, strong) NSLayoutConstraint *contentViewBottom;
@property double troubleBoxDefaultHeight;
@property (nonatomic, strong) FloatingErrorView *errorView;
@property (weak) IBOutlet NSView *errorContainerView;

@end

@implementation MainWindowController

extern void *ctx;

- (id)initWithWindow:(NSWindow *)window
{
	self = [super initWithWindow:window];
	if (self)
	{
		self.loginViewController = [[LoginViewController alloc]
									initWithNibName:@"LoginViewController" bundle:nil];
		self.timeEntryListViewController = [[TimeEntryListViewController alloc]
											initWithNibName:@"TimeEntryListViewController" bundle:nil];
		self.overlayViewController = [[OverlayViewController alloc]
									  initWithNibName:@"OverlayViewController" bundle:nil];


		[self.loginViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self.timeEntryListViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
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
	}
	return self;
}

- (void)windowDidLoad
{
	[super windowDidLoad];

	[self.window setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageNamed:@"background-pattern"]]];

	// Tracking the size of window after loaded
	[self trackWindowSize];

	// Error View
	[self initErrorView];
}

- (void)initErrorView {
	self.errorView = [FloatingErrorView initFromXib];
	self.errorView.translatesAutoresizingMaskIntoConstraints = NO;
	self.errorView.delegate = self;

	[self.errorContainerView addSubview:self.errorView];

	[self.errorContainerView addConstraint:[NSLayoutConstraint constraintWithItem:self.errorContainerView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.errorView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0]];
	[self.errorContainerView addConstraint:[NSLayoutConstraint constraintWithItem:self.errorContainerView attribute:NSLayoutAttributeLeading relatedBy:NSLayoutRelationEqual toItem:self.errorView attribute:NSLayoutAttributeLeading multiplier:1.0 constant:0]];
	[self.errorContainerView addConstraint:[NSLayoutConstraint constraintWithItem:self.errorContainerView attribute:NSLayoutAttributeTrailing relatedBy:NSLayoutRelationEqual toItem:self.errorView attribute:NSLayoutAttributeTrailing multiplier:1.0 constant:0]];
	[self.errorContainerView addConstraint:[NSLayoutConstraint constraintWithItem:self.errorContainerView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.errorView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0]];

	// Able to draw shadow
	self.errorContainerView.wantsLayer = YES;
	self.errorContainerView.layer.masksToBounds = NO;

	// Hidden by default
	self.errorContainerView.hidden = YES;
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
//        [self.loginViewController changeView:NO];
		[self.contentView addSubview:self.loginViewController.view];
		[self.loginViewController.view setFrame:self.contentView.bounds];
//        [self.loginViewController.email.window makeFirstResponder:self.loginViewController.email];

		[self.timeEntryListViewController.view removeFromSuperview];
		[self.overlayViewController.view removeFromSuperview];
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
			|| [self.timeEntryListViewController.view superview] == nil)
		{
			// Close error when loging in
			[self closeError];

			[self.contentView addSubview:self.timeEntryListViewController.view];
			[self.timeEntryListViewController.view setFrame:self.contentView.bounds];

			[self.loginViewController.view removeFromSuperview];
			[self.overlayViewController.view removeFromSuperview];

			[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kFocusTimer
																		object:nil];
		}
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
	[self.timeEntryListViewController.view removeFromSuperview];
}

- (void)startDisplayError:(NSNotification *)notification
{
	[self displayError:notification.object];
}

- (void)displayError:(NSString *)msg
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.errorContainerView.hidden = NO;
	NSString *errorMessage = msg == nil ? @"Error" : msg;
	[self.errorView updateWithError:errorMessage];
}

- (void)startDisplayOnlineState:(NSNotification *)notification
{
	[self displayOnlineState:notification.object];
}

- (void)displayOnlineState:(NSNumber *)status
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self setContentViewBottomConstraint];

	switch ([status intValue])
	{
		case 1 :
			[self.onlineStatusTextField setHidden:NO];
			[self.onlineStatusTextField setStringValue:@"Status: Offline, no network"];
			self.contentViewBottom.constant = -20;
			break;
		case 2 :
			[self.onlineStatusTextField setHidden:NO];
			[self.onlineStatusTextField setStringValue:@"Status: Offline, Toggl not responding"];
			self.contentViewBottom.constant = -20;
			break;
		default :
			[self.onlineStatusTextField setHidden:YES];
			[self.onlineStatusTextField setStringValue:@"Status: Online"];
			self.contentViewBottom.constant = 0;
			break;
	}
}

- (void)setContentViewBottomConstraint
{
	if (!self.contentViewBottom)
	{
		self.contentViewBottom = [NSLayoutConstraint constraintWithItem:self.contentView
															  attribute:NSLayoutAttributeBottom
															  relatedBy:NSLayoutRelationEqual
																 toItem:self.mainView
															  attribute:NSLayoutAttributeBottom
															 multiplier:1
															   constant:0];
		[self.mainView addConstraint:self.contentViewBottom];
	}
}

- (void)stopDisplayError:(NSNotification *)notification
{
	[self closeError];
}

- (void)closeError
{
	self.errorContainerView.hidden = YES;
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
	return self.timeEntryListViewController.timeEntrypopover.shown;
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

- (void)floatingErrorShouldHide {
	[self closeError];
}

@end
