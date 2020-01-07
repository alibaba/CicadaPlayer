//
// Created by moqi on 2019/12/12.
//

#ifndef CICADAMEDIA_NETWORKEVENTRECEIVER_H
#define CICADAMEDIA_NETWORKEVENTRECEIVER_H

#include "IEventReceiver.h"
#include <data_source/IDataSource.h>
#include <memory>

class NetWorkEventReceiver : public IEventReceiver {
public:
    explicit NetWorkEventReceiver(Listener &listener);

    virtual ~NetWorkEventReceiver();

    void poll(bool &exit) override;

private:
    std::unique_ptr<Cicada::IDataSource> mDataSource{};

};


#endif //CICADAMEDIA_NETWORKEVENTRECEIVER_H
