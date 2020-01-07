//
// Created by moqi on 2019-08-20.
//

#ifndef CICADA_PLAYER_MEDIATYPEINTERNAL_H
#define CICADA_PLAYER_MEDIATYPEINTERNAL_H

#include "AFMediaType.h"
#include <utils/CicadaType.h>

class CICADA_CPLUS_EXTERN streamMeta {
public:
    explicit streamMeta(const Stream_meta *meta);

    ~streamMeta();

    explicit operator  Stream_meta *();

private:
    Stream_meta mMeta{};

};


#endif //CICADA_PLAYER_MEDIATYPEINTERNAL_H
