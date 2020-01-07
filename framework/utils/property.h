//
// Created by moqi on 2018/11/27.
//

#ifndef CICADA_PLAYER_PROPERTY_H
#define CICADA_PLAYER_PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

const char* getProperty(const char *key);

/*
 * @key
 * ro.* : can't be modified after be set
 * protected.* : only can be modified by the thread first set
 * return 0 if ok, otherwise return -1
 */
int setProperty(const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#endif //CICADA_PLAYER_PROPERTY_H
