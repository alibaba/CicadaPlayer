//
// Created by moqi on 2020/5/22.
//

#ifndef CICADAMEDIA_SPSC_QUEUE_H
#define CICADAMEDIA_SPSC_QUEUE_H

#include <boost/lockfree/spsc_queue.hpp>
namespace Cicada {
    template<typename T>
    class SpscQueue : public boost::lockfree::spsc_queue<T> {
    public:
        SpscQueue(int size)
            : boost::lockfree::spsc_queue<T>(size){

              };
        size_t size()
        {
            return this->read_available();
        }
    };

}// namespace Cicada

#endif//CICADAMEDIA_SPSC_QUEUE_H
