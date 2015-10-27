
#import <Foundation/Foundation.h>

#import "toggl_api.h"

@interface AutotrackerRuleItem : NSObject
+ (NSMutableArray *)loadAll:(TogglAutotrackerRuleView *)first;
- (void)load:(TogglAutotrackerRuleView *)data;
@property int64_t ID;
@property (strong) NSString *Term;
@property (strong) NSString *ProjectAndTaskLabel;
@end


