
#import "AutotrackerRuleItem.h"

@implementation AutotrackerRuleItem

- (void)load:(TogglAutotrackerRuleView *)data
{
	self.ID = data->ID;
	self.Term = [NSString stringWithUTF8String:data->Term];
	self.ProjectAndTaskLabel = [NSString stringWithUTF8String:data->ProjectAndTaskLabel];
}

+ (NSMutableArray *)loadAll:(TogglAutotrackerRuleView *)first
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	TogglAutotrackerRuleView *it = first;

	while (it)
	{
		AutotrackerRuleItem *item = [[AutotrackerRuleItem alloc] init];
		[item load:it];
		[result addObject:item];
		it = it->Next;
	}
	return result;
}

@end
