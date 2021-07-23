//
// Created by pingkai on 2021/7/23.
//

#ifndef QALIYUNPLAYERTEST_APPLESUBTITLERENDER_H
#define QALIYUNPLAYERTEST_APPLESUBTITLERENDER_H

#include "ISubtitleRender.h"
namespace Cicada {
    class AppleCATextLayerRender;
    class AppleSubtitleRender : public ISubtitleRender {
    public:
        AppleSubtitleRender();
        ~AppleSubtitleRender() override;

        void setView(void *view) override;

        int show(const std::string &data) override;

        int hide(const std::string &data) override;

        int intHeader(const char *header) override;

    private:
        AppleCATextLayerRender *mRender{nullptr};
    };
}// namespace Cicada


#endif//QALIYUNPLAYERTEST_APPLESUBTITLERENDER_H
