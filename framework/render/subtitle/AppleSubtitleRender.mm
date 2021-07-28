//
// Created by pingkai on 2021/7/23.
//

#include "AppleSubtitleRender.h"
#include "AppleCATextLayerRender.h"
using namespace Cicada;
AppleSubtitleRender::AppleSubtitleRender()
{
    mRender = new AppleCATextLayerRender();
}
AppleSubtitleRender::~AppleSubtitleRender()
{
    delete mRender;
}
void AppleSubtitleRender::setView(void *view)
{
    mRender->setView(view);
}
int AppleSubtitleRender::show(int64_t index,const std::string &data)
{
    return mRender->show(index,data);
}
int AppleSubtitleRender::hide(int64_t index,const std::string &data)
{
    return mRender->hide(index,data);
}
int AppleSubtitleRender::intHeader(const char *header)
{
    return mRender->intHeader(header);
}
