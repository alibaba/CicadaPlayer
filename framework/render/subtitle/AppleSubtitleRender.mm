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
int AppleSubtitleRender::show(const std::string &data)
{
    return mRender->show(data);
}
int AppleSubtitleRender::hide(const std::string &data)
{
    return mRender->hide(data);
}
int AppleSubtitleRender::intHeader(const char *header)
{
    return mRender->intHeader(header);
}
