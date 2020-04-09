#import "CicadaPlayerGlobalSettings.h"
#import "utils/globalSettings.h"

@interface CicadaPlayerGlobalSettings ()
{
}
@end

@implementation CicadaPlayerGlobalSettings

+(void)setDNSResolve:(NSString *)host ip:(NSString *)ip
{
    if (nil == host) {
        return;
    }

    const string theHost = [host UTF8String];
    Cicada::globalSettings::getSetting()->removeResolve(theHost, "");

    string theip;
    if (nil != ip && 0 < [ip length]) {
        theip = [ip UTF8String];
        Cicada::globalSettings::getSetting()->addResolve(theHost, theip);
    }
}

@end
