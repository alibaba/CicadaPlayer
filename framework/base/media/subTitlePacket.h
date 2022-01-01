//
// Created by moqi on 2019/9/24.
//

#ifndef CICADA_PLAYER_SUBTITLEPACKET_H
#define CICADA_PLAYER_SUBTITLEPACKET_H

#include "IAFPacket.h"

class subTitlePacket : public IAFPacket {
public:
    subTitlePacket(uint8_t *data, int size, int64_t pts, int64_t duration);
//    subTitlePacket(uint8_t **data, int64_t pts, int size);

    ~subTitlePacket() override;

    std::unique_ptr<IAFPacket> clone() const override;

    uint8_t *getData() override;

    int64_t getSize() override;

    void setProtected() override
    {}

private:
    uint8_t *mpBuffer{};
    int64_t mSize;
};


#endif //CICADA_PLAYER_SUBTITLEPACKET_H
