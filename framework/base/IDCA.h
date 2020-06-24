//
// Created by moqi on 2020/6/24.
//

#ifndef CICADAMEDIA_IDCA_H
#define CICADAMEDIA_IDCA_H
#include <string>
// direct component access
namespace Cicada {
    class IDCAObserver {
    public:
        virtual void onEvent(int level, const std::string &content) = 0;
    };
    class IDCA {
    public:
        void setDCAObserver(IDCAObserver *observer)
        {
            mObserver = observer;
        }
        virtual int invoke(int cmd, const std::string &content) = 0;
        virtual ~IDCA() = default;

    protected:
        void sendEvent(int level, const std::string &content)
        {
            if (mObserver) {
                mObserver->onEvent(level, content);
            }
        };

    private:
        IDCAObserver *mObserver{nullptr};
    };
}// namespace Cicada
#endif//CICADAMEDIA_IDCA_H
