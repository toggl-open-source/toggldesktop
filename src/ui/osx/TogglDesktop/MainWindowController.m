//
//  MainWindowController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "MainWindowController.h"
#import "LoginViewController.h"
#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "Update.h"
#import "MenuItemTags.h"
#import "DisplayCommand.h"

@interface MainWindowController ()
@property (nonatomic, strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic, strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property NSLayoutConstraint *contentViewTop;

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

		self.isNetworkIssue = NO;

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

	self.isNetworkIssue = NO;

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

- (void)displayOnlineState:(NSString *)msg
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (msg)
	{
		self.isNetworkIssue = YES;

		[self.errorLabel setStringValue:msg];
		self.contentViewTop.constant = 50;
		[self.troubleBox setHidden:NO];
	}
	else if (self.isNetworkIssue)
	{
		self.isNetworkIssue = NO;
		[self closeError];
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

@end
