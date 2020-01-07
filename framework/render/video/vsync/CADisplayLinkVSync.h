//
// Created by moqi on 2019-09-05.
//

#ifndef CICADA_PLAYER_CADISPLAYLINKVSYNC_H
#define CICADA_PLAYER_CADISPLAYLINKVSYNC_H

#include "IVSync.h"

class CADisplayLinkVSync : public IVSync {
public:
    explicit CADisplayLinkVSync(Listener &listener);

    ~CADisplayLinkVSync() override;

    void start() override;

    void pause() override;

    float getHz() override {
        return 0;
    }

private:

    void *c;

};


#endif //CICADA_PLAYER_CADISPLAYLINKVSYNC_H
