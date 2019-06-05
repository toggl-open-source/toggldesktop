//
//  FeedbackWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "FeedbackWindowController.h"
#import "toggl_api.h"
#import "NSAlert+Utils.h"
#import "TogglDesktop-Swift.h"
#import "ConvertHexColor.h"

static NSInteger const kMaxFileSize = 5000000;

typedef enum : NSUInteger
{
	FeedbackErrorMissingTopic,
	FeedbackErrorMissingContent,
	FeedbackErrorFileIsLarge,
	FeedbackErrorInvalidFile,
	FeedbackErrorNone
} FeedbackError;

@interface FeedbackWindowController ()
@property (weak) IBOutlet NSComboBox *topicComboBox;
@property (unsafe_unretained) IBOutlet NSTextView *contentTextView;
@property (weak) IBOutlet FlatButton *uploadImageButton;
@property (weak) IBOutlet FlatButton *sendButton;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (weak) IBOutlet NSTextField *selectedImageTextField;
@property (weak) IBOutlet NSBox *selectedImageBox;
@property (copy, nonatomic) NSString *selectedFile;

- (IBAction)uploadImageClick:(id)sender;
- (IBAction)sendClick:(id)sender;
@end

@implementation FeedbackWindowController

extern void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	// Clean window titlebar
	self.window.titleVisibility = NSWindowTitleHidden;
	self.window.titlebarAppearsTransparent = YES;
	self.window.styleMask |= NSFullSizeContentViewWindowMask;
}

- (void)awakeFromNib
{
	[super awakeFromNib];

	self.uploadImageButton.wantsLayer = YES;
	self.uploadImageButton.layer.borderWidth = 1;
	if (@available(macOS 10.13, *))
	{
		self.uploadImageButton.layer.borderColor = [NSColor colorNamed:@"upload-border-color"].CGColor;
		self.uploadImageButton.bgColor = [NSColor colorNamed:@"upload-background-color"];
		self.contentTextView.backgroundColor = [NSColor colorNamed:@"upload-background-color"];
	}
	else
	{
		self.uploadImageButton.layer.borderColor = [ConvertHexColor hexCodeToNSColor:@"#acacac"].CGColor;
		self.uploadImageButton.bgColor = NSColor.whiteColor;
		self.contentTextView.backgroundColor = NSColor.whiteColor;
	}
}

- (IBAction)uploadImageClick:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];

	panel.allowsMultipleSelection = NO;
	panel.canChooseDirectories = NO;
	panel.canChooseFiles = YES;
	panel.allowedFileTypes = @[@"jpg", @"png", @"pdf", @"bmp", @"tiff"];
	__weak typeof(self) weakSelf = self;
	[panel beginWithCompletionHandler:^(NSInteger result) {
		 if (result == NSFileHandlingPanelOKButton)
		 {
			 NSURL *url = [[panel URLs] objectAtIndex:0];
			 FeedbackError error = [weakSelf validateFileSelectionWithURL:url];

	         // Set selected file
			 NSString *filePath = error == FeedbackErrorNone ? url.path : nil;
			 weakSelf.selectedFile = filePath;

	         // Show selected file
			 weakSelf.selectedImageBox.hidden = NO;
			 weakSelf.selectedImageTextField.stringValue = url.lastPathComponent;
			 weakSelf.selectedImageTextField.toolTip = filePath;

	         // Show error label
			 [weakSelf showAlertTitleWithError:error];
		 }
	 }];
}

- (IBAction)sendClick:(id)sender
{
	FeedbackError err = [self validateUserFeedback];

	// Show alert
	[self showAlertTitleWithError:err];

	// Focus
	switch (err)
	{
		case FeedbackErrorMissingTopic :
			[self.topicComboBox.window makeFirstResponder:self.topicComboBox];
			break;
		case FeedbackErrorMissingContent :
			[self.contentTextView.window makeFirstResponder:self.contentTextView];
			break;
		default :
			break;
	}

	// Early exit if it's error
	if (err != FeedbackErrorNone)
	{
		return;
	}

	// Send feedback
	if (!toggl_feedback_send(ctx,
							 [self.topicComboBox.stringValue UTF8String],
							 [self.contentTextView.string UTF8String],
							 [self.selectedFile UTF8String]))
	{
		[self showAlertTitleWithError:FeedbackErrorFileIsLarge];
		return;
	}

	// Finalize
	[self.window close];
	self.selectedImageBox.hidden = YES;
	[self.contentTextView setString:@""];
	[self.topicComboBox setStringValue:@""];

	[[NSAlert alertWithMessageText:@"Thank you!"
		 informativeTextWithFormat:@"Your feedback was sent successfully."] runModal];
}

- (NSString *)titleForError:(FeedbackError)feedbackError {
	switch (feedbackError)
	{
		case FeedbackErrorMissingTopic :
			return @"Feedback not sent – please choose a topic";

		case FeedbackErrorMissingContent :
			return @"Feedback not sent – please type in your feedback";

		case FeedbackErrorFileIsLarge :
			return @"Feedback not sent – please check that file you are sending is not larger than 5MB";

		case FeedbackErrorInvalidFile :
			return @"Feedback not sent – image file is invalid - please choose different file";

		case FeedbackErrorNone :
			return @"";
	}
}

- (FeedbackError)validateUserFeedback {
	if (self.topicComboBox.stringValue == nil
		|| [self.topicComboBox.stringValue isEqualToString:@""])
	{
		return FeedbackErrorMissingTopic;
	}

	if (self.contentTextView.string == nil
		|| [self.contentTextView.string isEqualToString:@""])
	{
		return FeedbackErrorMissingContent;
	}

	return FeedbackErrorNone;
}

- (FeedbackError)validateFileSelectionWithURL:(NSURL *)url {
	if (url == nil)
	{
		return FeedbackErrorInvalidFile;
	}

	NSError *err;
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSDictionary *attributesDict = [fileManager attributesOfItemAtPath:url.path error:&err];
	NSInteger fileSize = [attributesDict fileSize];

	NSLog(@"%ld", fileSize);

	if (err)
	{
		return FeedbackErrorInvalidFile;
	}

	if (fileSize > kMaxFileSize)
	{
		return FeedbackErrorFileIsLarge;
	}

	return FeedbackErrorNone;
}

- (void)showAlertTitleWithError:(FeedbackError)error {
	self.errorLabel.hidden = error == FeedbackErrorNone;
	self.errorLabel.stringValue = [self titleForError:error];
}

- (IBAction)closeFileOnTap:(id)sender {
	self.errorLabel.hidden = YES;
	self.selectedImageBox.hidden = YES;
	self.selectedFile = nil;
}

@end
