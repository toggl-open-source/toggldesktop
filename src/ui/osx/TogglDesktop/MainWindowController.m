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
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "MenuItemTags.h"
#import "DisplayCommand.h"
#include <Carbon/Carbon.h>

@interface MainWindowController ()
@property (nonatomic, strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic, strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property NSLayoutConstraint *contentViewTop;
@property NSLayoutConstraint *contentViewBottom;

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


		[self.loginViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self.timeEntryListViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayLogin:)
													 name:kDisplayLogin
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryList:)
													 name:kDisplayTimeEntryList
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

	[self.window setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageNamed:@"toggl-desktop-bg.png"]]];

	NSMutableAttributedString *attrTitle =
		[[NSMutableAttributedString alloc] initWithAttributedString:[self.closeTroubleBoxButton attributedTitle]];
	NSRange range = NSMakeRange(0, [attrTitle length]);
	[attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor whiteColor] range:range];
	[attrTitle fixAttributesInRange:range];
	[self.closeTroubleBoxButton setAttributedTitle:attrTitle];
}

- (void)addErrorBoxConstraint
{
	if (!self.contentViewTop)
	{
		self.contentViewTop = [NSLayoutConstraint constraintWithItem:self.contentView
														   attribute:NSLayoutAttributeTop
														   relatedBy:NSLayoutRelationEqual
															  toItem:self.mainView
														   attribute:NSLayoutAttributeTop
														  multiplier:1
															constant:0];
		[self.mainView addConstraint:self.contentViewTop];
	}
	self.contentViewTop.constant = 0;
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayLogin:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		[self addErrorBoxConstraint];
		[self.contentView addSubview:self.loginViewController.view];
		[self.loginViewController.view setFrame:self.contentView.bounds];
		[self.loginViewController.email becomeFirstResponder];

		[self.timeEntryListViewController.view removeFromSuperview];
	}
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeEntryList:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		// Close error when loging in
		if ([self.loginViewController.view superview] != nil)
		{
			[self closeError];
		}
		[self addErrorBoxConstraint];
		[self.contentView addSubview:self.timeEntryListViewController.view];
		[self.timeEntryListViewController.view setFrame:self.contentView.bounds];

		[self.loginViewController.view removeFromSuperview];
	}
}

- (void)startDisplayError:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayError:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayError:(NSString *)msg
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self.errorLabel setStringValue:msg];
	self.contentViewTop.constant = 50;
	[self.troubleBox setHidden:NO];
}

- (void)startDisplayOnlineState:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayOnlineState:)
						   withObject:notification.object
						waitUntilDone:NO];
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
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self.troubleBox setHidden:YES];
	self.contentViewTop.constant = 0;
	[self.errorLabel setStringValue:@""];
}

- (IBAction)errorCloseButtonClicked:(id)sender
{
	[self closeError];
}

- (void)keyUp:(NSEvent *)event
{
	if ([event keyCode] == kVK_DownArrow && ([event modifierFlags] & NSShiftKeyMask))
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kFocusListing
															object:nil
														  userInfo:nil];
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

@end
