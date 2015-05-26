
#import <Foundation/Foundation.h>

#import "toggl_api.h"

@interface AutotrackerRuleItem : NSObject
+ (NSMutableArray *)loadAll:(TogglAutotrackerRuleView *)first;
- (void)load:(TogglAutotrackerRuleView *)data;
@property int64_t ID;
@property uint64_t PID;
@property (strong) NSString *Term;
@property (strong) NSString *ProjectName;
@end


