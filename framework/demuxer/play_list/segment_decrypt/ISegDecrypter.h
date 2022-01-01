//
// Created by moqi on 2018/10/23.
//

#ifndef CICADA_PLAYER_ISEGDECRYPTER_H
#define CICADA_PLAYER_ISEGDECRYPTER_H


#include <cstdint>
#include <string>

class ISegDecrypter {
public:
    typedef int (*read_cb)(void *arg, uint8_t *buffer, int size);

public:
    ISegDecrypter(read_cb read, void *arg)
    {
        mReadCb = read;
        mReadCbArg = arg;
    }

    virtual ~ISegDecrypter() = default;

    virtual int Read(uint8_t *buffer, int size) = 0;

    virtual void SetOption(const char *key, uint8_t *buffer, int size) = 0;

    virtual std::string GetOption(const std::string & key){
            return "";
    };

    virtual void flush() = 0;

protected:
    read_cb mReadCb = nullptr;
    void *mReadCbArg = nullptr;

};


#endif //CICADA_PLAYER_ISEGDECRYPTOR_H
