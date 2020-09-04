//
// Created by lifujun on 2020/8/10.
//

#ifndef SOURCE_QUERYLISTENER_H
#define SOURCE_QUERYLISTENER_H

#include "MediaPlayer.h"
#include "analytics/AnalyticsQueryListener.h"

namespace Cicada {
    class QueryListener : public AnalyticsQueryListener {
    public:
        explicit QueryListener(MediaPlayer *player);

        ~QueryListener() override = default;

        void setMediaPlayer(MediaPlayer *player);

        // analytics query interface
        int64_t OnAnalyticsGetCurrentPosition() override;

        int64_t OnAnalyticsGetBufferedPosition() override;

        int64_t OnAnalyticsGetDuration() override;

        std::string OnAnalyticsGetPropertyString(PropertyKey key) override;

    private:
        MediaPlayer *mPlayer = nullptr;
    };
}

#endif //SOURCE_QUERYLISTENER_H
