
#import "CicadaSyncPlayerServer.h"
#include <communication/messageServer.h>
#include "playerMessage.h"

using namespace Cicada;
using namespace std;
class syncServerListener;

@interface CicadaSyncPlayerServer()<CicadaDelegate>

@property (nonatomic, assign)CicadaStatus playerStatus;
@property (nonatomic, assign)BOOL isLoading;
@property (nonatomic, assign)BOOL isPrepare;
@property (nonatomic, assign)syncServerListener *syncListener;
@property (nonatomic, assign)messageServer *server;

@end

static NSString *serverIp = @"tcp://localhost:8888";
class syncServerListener : public IProtocolServer::Listener {
public:
    explicit syncServerListener(CicadaSyncPlayerServer *player) : mPlayer(player)
    {}

    void onAccept(IProtocolServer::IClient **client) override
    {
        if (mPlayer.playerStatus >= CicadaStatusPrepared) {
            Cicada::messageServer::write(playerMessage::seekAccurate, *client);
            Cicada::messageServer::write(to_string([mPlayer currentPosition]), *client);
            Cicada::messageServer::write(playerMessage::prepare, *client);
        }

        if (mPlayer.playerStatus == CicadaStatusStarted && !mPlayer.isLoading) {
            Cicada::messageServer::write(playerMessage::start, *client);
        }
    }

private:
    CicadaSyncPlayerServer *mPlayer = nil;
};

@implementation CicadaSyncPlayerServer

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.isPrepare = NO;
        self.autoPlay = YES;
        [super setInnerDelegate:self];

        self.syncListener = new syncServerListener(self);
        self.server = new messageServer(self.syncListener);
        self.server->init();
    }

    return self;
}


- (void)prepare
{
    [super prepare];
    self.isPrepare = YES;
}

- (void)stop
{
    [super stop];
    self.isPrepare = NO;
}

#pragma mark CicadaDelegate

-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType {
    switch (eventType) {
        case CicadaEventLoadingStart:
            self.isLoading = YES;
            if (self.server) {
                self.server->write(playerMessage::pause);
            }
            break;
        case CicadaEventLoadingEnd:
            self.isLoading = NO;
            if (self.server) {
                self.server->write(playerMessage::start);
            }
            break;
        default:
            break;
    }
}

-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    switch (eventWithString) {
        case CICADA_EVENT_PLAYER_NETWORK_RETRY:
            [player reload];
            break;
        default:
            break;
    }
}

- (void)onPlayerStatusChanged:(CicadaPlayer*)player oldStatus:(CicadaStatus)oldStatus newStatus:(CicadaStatus)newStatus {
    self.playerStatus = newStatus;
    switch (newStatus) {
        case CicadaStatusStarted:
            if (self.server) {
                self.server->write(playerMessage::start);
            }
            break;
        case CicadaStatusPaused:
            if (self.server) {
                self.server->write(playerMessage::pause);
            }
            break;
        default:
            break;
    }
}

- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    if (self.server) {
        self.server->write(playerMessage::clock);
        self.server->write(to_string([self getPlayingPts]));
    }
}

-(void)destroy
{
    [super destroy];
    delete self.server;
}

@end
