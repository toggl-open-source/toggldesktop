
#import <Foundation/Foundation.h>

@interface AutotrackerRuleItem : NSObject
@property (nonatomic, assign) NSInteger ID;
@property (nonatomic, copy) NSString *Term;
@property (nonatomic, copy) NSString *ProjectAndTaskLabel;

+ (NSMutableArray *)loadAll:(TogglAutotrackerRuleView *)first;
- (void)load:(TogglAutotrackerRuleView *)data;
@end


