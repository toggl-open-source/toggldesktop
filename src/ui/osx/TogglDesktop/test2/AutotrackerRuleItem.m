
#import "AutotrackerRuleItem.h"

@implementation AutotrackerRuleItem

- (void)load:(TogglAutotrackerRuleView *)data
{
	self.ID = data->ID;
	self.PID = data->PID;
	self.Term = [NSString stringWithUTF8String:data->Term];
	self.ProjectName = [NSString stringWithUTF8String:data->ProjectName];
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
