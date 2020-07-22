//
// Created by moqi on 2020/7/20.
//

#include "CicadaPlayerPrototype.h"
#include "SuperMediaPlayer.h"
#ifdef __APPLE__
#include "externalPlayer/AppleAVPlayer.h"
#endif
using namespace Cicada;
CicadaPlayerPrototype *CicadaPlayerPrototype::playerQueue[];
int CicadaPlayerPrototype::_nextSlot;
void CicadaPlayerPrototype::addPrototype(Cicada::CicadaPlayerPrototype *se)
{
    playerQueue[_nextSlot++] = se;
}
ICicadaPlayer *CicadaPlayerPrototype::create(const options *opts = nullptr)
{
    int score_res = 0;
    CicadaPlayerPrototype *playerType = nullptr;

    for (int i = 0; i < _nextSlot; ++i) {
        int score = playerQueue[i]->probeScore(opts);

        if (score > score_res) {
            score_res = score;
            playerType = playerQueue[i];

            if (score >= SUPPORT_MAX) {
                break;
            }
        }
    }

    if (playerType && score_res > SUPPORT_NOT) {
        ICicadaPlayer *player = playerType->clone();
        return player;
    }
#ifdef __APPLE__
    if (AppleAVPlayer::is_supported(opts)) {
        return new AppleAVPlayer();
    }
#endif
    return new SuperMediaPlayer();
}
