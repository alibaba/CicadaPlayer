
#import "CicadaSyncPlayerClient.h"
#include <communication/messageServer.h>
#include "playerMessage.h"
#include <utils/af_clock.h>
#include <utils/frame_work_log.h>

using namespace Cicada;
using namespace std;
class syncServerListener;

@interface CicadaSyncPlayerClient()<CicadaDelegate>
{
    af_clock masterClock;
}

@property (nonatomic, assign)bool isConnet;
@property (nonatomic, assign)messageClient *client;
@property (nonatomic, strong)NSTimer *mTimer;
@end

static string serverIp = "tcp://localhost:8888";

@implementation CicadaSyncPlayerClient

- (void)timerAction
{
    if (!self.isConnet) {
        self.isConnet = self.client->connect(serverIp) >= 0;
    } else {
        string msg = self.client->readMessage();
        if (!msg.empty()) {
            AF_LOGE("client msg %s\n", msg.c_str());
            if (msg == playerMessage::start) {
                [self start];
                masterClock.start();
            } else if (msg == playerMessage::pause) {
                [self pause];
                masterClock.pause();
            } else if (msg == playerMessage::prepare) {
                [self prepare];
            } else if (msg == playerMessage::clock) {
                int64_t pts = atoll(self.client->readMessage().c_str());
                masterClock.set(pts);
                if (llabs(pts - [self getPlayingPts]) > 40000) {
                    AF_LOGW("delta pts is %lld\n", pts - [self getPlayingPts]);
                }
            } else if (msg == playerMessage::seekAccurate) {
                int64_t ms = atoll(self.client->readMessage().c_str());
                [self seekToTime:ms seekMode:(CICADA_SEEKMODE_ACCURATE)];
            } else if (msg == playerMessage::seek) {
                int64_t ms = atoll(self.client->readMessage().c_str());
                [self seekToTime:ms seekMode:(CICADA_SEEKMODE_INACCURATE)];
            }
        }
    }
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.autoPlay = YES;
        [super setInnerDelegate:self];

        [self setPlaybackType:CicadaPlaybackTypeVideo];
        [self SetClockRefer:^int64_t{
            return masterClock.get();
        }];
        self.client = new messageClient();
        int ret = self.client->connect(serverIp);
        if (ret >= 0) {
            self.isConnet = true;
        } else {
            self.isConnet = false;
        }

        self.mTimer = [NSTimer timerWithTimeInterval:0.01 target:self selector:@selector(timerAction) userInfo:nil repeats:YES];
        [[NSRunLoop mainRunLoop] addTimer:self.mTimer forMode:NSDefaultRunLoopMode];
        [self.mTimer fire];
    }

    return self;
}

#pragma mark CicadaDelegate
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    switch (eventWithString) {
        case CICADA_EVENT_PLAYER_NETWORK_RETRY:
            [player reload];
            break;
        default:
            break;
    }
}

-(void)destroy
{
    if (nil != self.mTimer) {
        [self.mTimer invalidate];
        self.mTimer = nil;
    }
    [super destroy];
    delete self.client;
}

@end
