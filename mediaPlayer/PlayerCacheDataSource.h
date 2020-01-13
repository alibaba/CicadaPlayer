//
// Created by lifujun on 2019/9/24.
//

#ifndef SOURCE_CICADACACHEDATASOURCE_H
#define SOURCE_CICADACACHEDATASOURCE_H


#include <cacheModule/CacheManager.h>

class PlayerCacheDataSource : public ICacheDataSource{
public:
    explicit PlayerCacheDataSource(void *playerHandle);

    ~PlayerCacheDataSource() override;

    int64_t getStreamSize() override;

    int64_t getDuration() override;

    int getStreamMeta(Stream_meta *ptr, StreamType type) override;

private:
    void *mPlayerHandle = nullptr;
};


#endif //SOURCE_CICADACACHEDATASOURCE_H
