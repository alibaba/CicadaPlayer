//
// Created by moqi on 2019/12/12.
//

#define LOG_TAG "NetWorkEventReceiver"

#include "NetWorkEventReceiver.h"
#include <data_source/dataSourcePrototype.h>
#include <utils/frame_work_log.h>

using namespace Cicada;

NetWorkEventReceiver::NetWorkEventReceiver(IEventReceiver::Listener &listener) : IEventReceiver(listener)
{
    mDataSource = std::unique_ptr<Cicada::IDataSource>(dataSourcePrototype::create("udp://localhost:8090"));

    if (mDataSource) {
        int ret = mDataSource->Open("udp://localhost:8090");

        if (ret < 0) {
            mDataSource = nullptr;
        }
    }
}

NetWorkEventReceiver::~NetWorkEventReceiver()
{
    if (mDataSource) {
        mDataSource->Close();
    }
}

void NetWorkEventReceiver::poll(bool &exit)
{
    if (mDataSource == nullptr) {
        return;
    }

    char c = 0;
    int ret = mDataSource->Read(&c, 1);

    if (ret > 0 && c > 0) {
        switch (c) {
            case 'q':
                mListener.onExit();
                exit = true;
                break;

            case ' ':
                mListener.onPausePlay();
                break;

            case '>':
            case '<':
                mListener.onStepSeek(c == '>');
                break;

            case '+':
            case '-':
                mListener.onSpeedUp(c == '+');
                break;

            case 'V':
            case 'v':
                mListener.onChangeVolume(c == 'V');
                break;

            default:
                if (c >= '0' && c <= '9') {
                    mListener.onPercentageSeek((c - '0') * 10);
                }
                break;
        }
    }
}
