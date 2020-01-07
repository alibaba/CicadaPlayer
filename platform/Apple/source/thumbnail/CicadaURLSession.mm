
#import "CicadaURLSession.h"

@interface CicadaURLSession()
{

}
@property (nonatomic, strong) NSURLSession *session;
@end

@implementation CicadaURLSession

- (instancetype _Nonnull )init
{
    if (nil != (self = [super init]))
    {

    }

    return self;
}

- (void) request:(NSURL *_Nonnull)URL
        complete:(void (^_Nullable)(CicadaURLSession * _Nullable session, NSData * _Nullable data))complete
{
    self.session = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:self delegateQueue:[[NSOperationQueue alloc] init]];

    NSURLSessionDataTask *task =  [self.session dataTaskWithURL:URL completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        if (complete) {
            complete(self, data);
        }
    }];
    [task resume];
}

- (void) cancel
{
    [self.session finishTasksAndInvalidate];
    self.session = nil;
}

- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential *))completionHandler
{
    NSURLSessionAuthChallengeDisposition disposition = NSURLSessionAuthChallengePerformDefaultHandling;
    __block NSURLCredential *credential = nil;
    if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        disposition = NSURLSessionAuthChallengeUseCredential;
        credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
    } else {
        disposition = NSURLSessionAuthChallengePerformDefaultHandling;
    }

    if (completionHandler) {
        completionHandler(disposition, credential);
    }
}

-(void)dealloc
{
}

@end
