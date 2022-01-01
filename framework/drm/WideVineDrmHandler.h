//
// Created by SuperMan on 11/27/20.
//

#ifndef SOURCE_WIDEVINEDRMHANDLER_H
#define SOURCE_WIDEVINEDRMHANDLER_H

#include <jni.h>
#include <mutex>
#include "DrmHandler.h"
#include "DrmHandlerPrototype.h"

#define SESSION_STATE_ERROR (-1)
#define SESSION_STATE_IDLE (-2)
#define SESSION_STATE_OPENED (0)

namespace Cicada {
    class WideVineDrmHandler : public DrmHandler, private DrmHandlerPrototype {
    public:
        WideVineDrmHandler(const DrmInfo &drmInfo);

        ~WideVineDrmHandler() override;

        DrmHandler *clone(const DrmInfo &drmInfo) override;

        bool is_supported(const DrmInfo &drmInfo) override;

        bool isErrorState() override;

    public:
        int getState();

        int getErrorCode();

        bool isForceInsecureDecoder();

        void open();

        int getSessionId(char** sessionId);

        static void convertData(int naluLengthSize, uint8_t **new_data, int *new_size,
                                             const uint8_t *data,
                                             int size);
    public:
        static void init(JNIEnv *env);

        static void unInit(JNIEnv *env);

        static int registerMethod(JNIEnv *env);

        static jbyteArray
        requestProvision(JNIEnv *env, jobject instance, jlong nativeInstance, jstring url,
                         jbyteArray data);

        static jbyteArray
        requestKey(JNIEnv *env, jobject instance, jlong nativeInstance, jstring url,
                   jbyteArray data);

        static void changeState(JNIEnv *env, jobject instance, jlong nativeInstance,
                                jint state, jint errorCode);

        static void updateSessionId(JNIEnv *env, jobject instance, jlong nativeInstance,
                                    jbyteArray jSessionId);

    protected:
        explicit WideVineDrmHandler(int dummy)
                : DrmHandler(DrmInfo{}) {
            addPrototype(this);
        }

        static WideVineDrmHandler dummyWideVineHandler;

    private:
        jobject mJDrmSessionManger = nullptr;

        std::mutex mDrmMutex{};

        char *mSessionId = nullptr;
        int mSessionSize{0};

        int mState{SESSION_STATE_IDLE};
        int mErrorCode{0};

        bool bSessionRequested{false};
    };
}


#endif //SOURCE_WIDEVINEDRMHANDLER_H
