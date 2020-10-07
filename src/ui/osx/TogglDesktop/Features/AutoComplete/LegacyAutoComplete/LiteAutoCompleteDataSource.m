//
//  LiteAutoCompleteDataSource.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 23/02/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "LiteAutoCompleteDataSource.h"
#import "AutoCompleteTableCell.h"
#import "AutoCompleteTable.h"
#import "Toggl_Track-Swift.h"
#import "AutoCompleteInput.h"
#import "AutocompleteItem.h"

@interface LiteAutoCompleteDataSource () <NSTableViewDataSource, AutoCompleteInputDelegate>
@property (assign, nonatomic) CGFloat itemHeight;
@property (assign, nonatomic) CGFloat worksapceItemHeight;
@property (nonatomic, strong) NSMutableArray *orderedKeys;
@property (nonatomic, strong) NSMutableArray *filteredOrderedKeys;
@property (nonatomic, assign) NSInteger textLength;
@property (nonatomic, assign) NSInteger lastType;
@property (nonatomic, assign) NSInteger lastWID;
@property (nonatomic, copy) NSString *lastClientLabel;
@property (nonatomic, strong) NSArray *types;
@end

@implementation LiteAutoCompleteDataSource

extern void *ctx;

- (id)initWithNotificationName:(NSString *)notificationName
{
	self = [super init];

	self.orderedKeys = [[NSMutableArray alloc] init];
	self.filteredOrderedKeys = [[NSMutableArray alloc] init];
	self.lastType = -1;
	self.lastWID = -1;
	self.lastClientLabel = nil;
	self.types = [NSArray arrayWithObjects:@"TIME ENTRIES", @"TASKS", @"PROJECTS", @"WORKSPACES", nil];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayAutocomplete:)
												 name:notificationName
											   object:nil];
	return self;
}

- (NSString *)completedString:(NSString *)partialString
{
	@synchronized(self)
	{
		for (NSString *text in self.filteredOrderedKeys)
		{
			if ([[text commonPrefixWithString:partialString
									  options:NSCaseInsensitiveSearch] length] == [partialString length])
			{
				return text;
			}
		}
	}
	return @"";
}

- (AutocompleteItem *)itemAtIndex:(NSInteger)row
{
	AutocompleteItem *item;

	@synchronized(self)
	{
		item = [self.filteredOrderedKeys objectAtIndex:row];
	}

	if (item.Type == -1)
	{
		return nil;
	}
	return item;
}

- (void)startDisplayAutocomplete:(NSNotification *)notification
{
	[self displayAutocomplete:notification.object];
}

- (void)displayAutocomplete:(NSMutableArray *)entries
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

    self.lastType = -1;
    self.lastClientLabel = nil;
    self.lastWID = -1;
    BOOL noProjectAdded = NO;
    [self.orderedKeys removeAllObjects];
    for (AutocompleteItem *item in entries)
    {
        // Add workspace title
        if (item.WorkspaceID != self.lastWID
            && item.WorkspaceName != nil)
        {
            AutocompleteItem *it = [[AutocompleteItem alloc] init];
            it.Type = -3;
            it.Text = item.WorkspaceName;
            [self addItem:it];
            self.lastWID = item.WorkspaceID;
            self.lastType = -1;
            self.lastClientLabel = nil;
        }

        // Add category title
        if (item.Type != self.lastType && item.Type != 1)
        {
            AutocompleteItem *it = [[AutocompleteItem alloc] init];
            it.Type = -1;
            it.Text = self.types[item.Type];
            [self addItem:it];
            self.lastType = item.Type;

            // Add 'No project' item
            if (item.Type == 2 && !noProjectAdded)
            {
                AutocompleteItem *it = [[AutocompleteItem alloc] init];
                it.Type = 2;
                it.Text = @"No project";
                it.ProjectAndTaskLabel = @"";
                [self addItem:it];
                noProjectAdded = YES;
            }
        }

        // Add Client name
        if (item.Type == 2 &&
            ![item.ClientLabel isEqualToString:self.lastClientLabel])
        {
            AutocompleteItem *it = [[AutocompleteItem alloc] init];
            it.Type = -2;
            it.Text = item.ClientLabel;
            if (it.Text.length == 0)
            {
                it.Text = @"No Client";
            }
            [self addItem:it];
            self.lastClientLabel = item.ClientLabel;
        }
        [self addItem:item];
    }

    if (self.input.autocompleteTableView.dataSource == nil)
    {
        self.input.autocompleteTableView.dataSource = self;
    }

    [self setFilter:self.currentFilter];
}

- (void)addItem:(AutocompleteItem *)item
{
	[self.orderedKeys addObject:item];
}

- (void)reload
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self.input reloadAutocomplete:self.filteredOrderedKeys];
}

- (void)findFilter:(NSString *)filter
{
	self.lastType = -1;
	self.lastClientLabel = nil;
	self.lastWID = -1;

    NSInteger lastPID = -1;
    NSMutableArray *filtered = [[NSMutableArray alloc] init];
    NSArray *stringArray = [filter componentsSeparatedByString:@" "];
    for (int i = 0; i < self.orderedKeys.count; i++)
    {
        AutocompleteItem *item = self.orderedKeys[i];
        // Skip filtering category/client/workspace items
        if (item.Type < 0 || (item.Type == 2 && item.ProjectID == 0))
        {
            continue;
        }
        NSString *key = (item.Type == 1) ? item.ProjectAndTaskLabel : item.Text;

        if (stringArray.count > 1)
        {
            // Filter is more than 1 word. Let's search for all the words entered.
            int foundCount = 0;
            for (int j = 0; j < stringArray.count; j++)
            {
                NSString *splitFilter = stringArray[j];

                if ([key rangeOfString:splitFilter options:NSCaseInsensitiveSearch].location != NSNotFound)
                {
                    foundCount++;
                    if ([key length] > self.textLength)
                    {
                        self.textLength = [key length];
                    }

                    if (foundCount == stringArray.count)
                    {
                        // Add workspace title
                        if (item.WorkspaceID != self.lastWID
                            && item.WorkspaceName != nil)
                        {
                            AutocompleteItem *it = [[AutocompleteItem alloc] init];
                            it.Type = -3;
                            it.Text = item.WorkspaceName;
                            [filtered addObject:it];
                            self.lastWID = item.WorkspaceID;
                            self.lastType = -1;
                            self.lastClientLabel = nil;
                        }

                        // Add category title
                        int64_t itemType = (item.Type == 1) ? 2 : item.Type;
                        if (itemType != self.lastType)
                        {
                            AutocompleteItem *it = [[AutocompleteItem alloc] init];
                            it.Type = -1;
                            it.Text = self.types[itemType];
                            [filtered addObject:it];
                            self.lastType = itemType;
                            lastPID = -1;
                        }

                        // Add client name row
                        if ((item.Type == 2 || item.Type == 1)
                            && ![item.ClientLabel isEqual:self.lastClientLabel])
                        {
                            AutocompleteItem *it = [[AutocompleteItem alloc] init];
                            it.Type = -2;
                            it.Text = item.ClientLabel;
                            if (it.Text.length == 0)
                            {
                                it.Text = @"No Client";
                            }
                            [filtered addObject:it];
                            self.lastClientLabel = item.ClientLabel;
                        }

                        // In case we have task and project is not completed
                        if (item.Type == 1 && item.ProjectID != lastPID)
                        {
                            AutocompleteItem *it = [[AutocompleteItem alloc] init];
                            it.Type = 2;
                            it.Text = item.ProjectLabel;
                            it.ProjectLabel = item.ProjectLabel;
                            it.ProjectColor = item.ProjectColor;
                            it.ProjectID = item.ProjectID;
                            it.Description = item.Description;
                            it.TaskLabel = @"";
                            it.ClientLabel = item.ClientLabel;
                            it.ProjectAndTaskLabel = item.ProjectAndTaskLabel;
                            [filtered addObject:it];
                        }
                        lastPID = item.ProjectID;

                        [filtered addObject:item];
                    }
                }
            }
        }
        else
        {
            // Single word filter
            if ([key rangeOfString:filter options:NSCaseInsensitiveSearch].location != NSNotFound)
            {
                if ([key length] > self.textLength)
                {
                    self.textLength = [key length];
                }
                // Add workspace title
                if (item.WorkspaceID != self.lastWID
                    && item.WorkspaceName != nil)
                {
                    AutocompleteItem *it = [[AutocompleteItem alloc] init];
                    it.Type = -3;
                    it.Text = item.WorkspaceName;
                    [filtered addObject:it];
                    self.lastWID = item.WorkspaceID;
                    self.lastType = -1;
                    self.lastClientLabel = nil;
                }

                // Add category title
                int64_t itemType = (item.Type == 1) ? 2 : item.Type;
                if (itemType != self.lastType)
                {
                    AutocompleteItem *it = [[AutocompleteItem alloc] init];
                    it.Type = -1;
                    it.Text = self.types[itemType];
                    [filtered addObject:it];
                    self.lastType = itemType;
                    lastPID = -1;
                }

                // Add client name row
                if ((item.Type == 2 || item.Type == 1)
                    && ![item.ClientLabel isEqual:self.lastClientLabel])
                {
                    AutocompleteItem *it = [[AutocompleteItem alloc] init];
                    it.Type = -2;
                    it.Text = item.ClientLabel;
                    if (it.Text.length == 0)
                    {
                        it.Text = @"No Client";
                    }
                    [filtered addObject:it];
                    self.lastClientLabel = item.ClientLabel;
                }

                // In case we have task and project is not completed
                if (item.Type == 1 && item.ProjectID != lastPID)
                {
                    AutocompleteItem *it = [[AutocompleteItem alloc] init];
                    it.Type = 2;
                    it.Text = item.ProjectLabel;
                    it.ProjectLabel = item.ProjectLabel;
                    it.ProjectColor = item.ProjectColor;
                    it.ProjectID = item.ProjectID;
                    it.Description = item.Description;
                    it.TaskLabel = @"";
                    it.ClientLabel = item.ClientLabel;
                    it.ProjectAndTaskLabel = item.ProjectAndTaskLabel;
                    [filtered addObject:it];
                }
                lastPID = item.ProjectID;
                [filtered addObject:item];
            }
        }
    }

    self.filteredOrderedKeys = filtered;
    [self reload];
}

- (void)clearFilter
{
	self.currentFilter = nil;
	self.filteredOrderedKeys = [[NSMutableArray alloc] init];
}

- (void)setFilter:(NSString *)filter
{
    if ([filter isEqualToString:self.currentFilter]) {
        return;
    }

    self.textLength = 0;
    bool lastFilterWasNonEmpty = ((filter == nil || filter.length == 0) && (self.currentFilter != nil && self.currentFilter.length > 0));

    self.currentFilter = filter;
    if (filter == nil || filter.length == 0)
    {
        self.filteredOrderedKeys = [NSMutableArray arrayWithArray:self.orderedKeys];
        if (lastFilterWasNonEmpty)
        {
            [self reload];
        }
        else
        {
            [self.input.autocompleteTableView reloadData];
            CGFloat totalHeight = [self.input calculateTotalHeightFromArray:self.filteredOrderedKeys];
            [self.input updateDropdownWithHeight:totalHeight];
        }
        return;
    }
    NSString *trimmedFilter = [filter stringByTrimmingCharactersInSet:
                               [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    [self findFilter:trimmedFilter];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
	NSUInteger result = 0;

	@synchronized(self)
	{
		result = [self.filteredOrderedKeys count];
	}

	NSLog(@"----- ROWS: %lu", (unsigned long)result);

	return result;
}

- (void)setInput:(AutoCompleteInput *)input
{
	_input = input;
	_input.inputDelegate = self;
    _input.autocompleteTableView.dataSource = self;
}

#pragma mark - AutoCompleteInputDelegate

- (void)autoCompleteInputShouldResetFilter
{
	[self clearFilter];
}

@end

