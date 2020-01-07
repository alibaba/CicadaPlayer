//
// Created by lifujun on 2019/11/1.
//

#ifndef SOURCE_IMUXERPROTOTYPE_H
#define SOURCE_IMUXERPROTOTYPE_H


#include "IMuxer.h"

class CICADA_CPLUS_EXTERN IMuxerPrototype {
    static IMuxerPrototype *muxerPrototypeQueue[10];
    static int _nextSlot;
public:
    virtual ~IMuxerPrototype() = default;

    static IMuxer* create(const string& destPath , const string& destFormat, const string& description);

protected:
    static void addPrototype(IMuxerPrototype *se);

private:
    virtual IMuxer *clone(const string& destPath , const string& destFormat, const string& description) = 0;

    virtual bool is_supported(const string& destPath , const string& destFormat, const string& description) = 0;

};


#endif //SOURCE_IMUXERPROTOTYPE_H
