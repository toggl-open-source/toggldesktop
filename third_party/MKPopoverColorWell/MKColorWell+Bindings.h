//
//  MKColorWell+Bindings.h
//  MKPopoverColorWell
//
//  via http://www.tomdalling.com/blog/cocoa/implementing-your-own-cocoa-bindings
//

#import "MKColorWell.h"

@interface MKColorWell (Bindings)
-(void)propagateValue:(id)value
           forBinding:(NSString*)binding;
@end
