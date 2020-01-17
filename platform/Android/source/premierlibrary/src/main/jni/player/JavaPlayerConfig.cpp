//
// Created by lifujun on 2018/12/7.
//

#include <string>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/GetObjectField.h>
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/CallObjectMethod.h>
#include <utils/Android/FindClass.h>
#include "JavaPlayerConfig.h"


static const char *playerConfigPath = "com/cicada/player/nativeclass/PlayerConfig";


//PLAYER_CONFIG
jclass    gj_PlayerConfig_class                     = nullptr;
jmethodID gj_PlayerConfig_Init                      = nullptr;
jmethodID gj_playerconfig_getCustomHeaders          = nullptr;
jmethodID gj_playerconfig_setCustomHeaders          = nullptr;
jfieldID  gj_playerconfig_HttpProxy                 = nullptr;
jfieldID  gj_playerconfig_Referrer                  = nullptr;
jfieldID  gj_playerconfig_NetworkTimeout            = nullptr;
jfieldID  gj_playerconfig_MaxDelayTime              = nullptr;
jfieldID  gj_playerconfig_MaxBufferedPacketDuration = nullptr;
jfieldID  gj_playerconfig_HighBufferLevel           = nullptr;
jfieldID  gj_playerconfig_FirstStartBufferLevel     = nullptr;
jfieldID  gj_playerconfig_MaxProbeSize              = nullptr;
jfieldID  gj_playerconfig_ClearFrameWhenStop        = nullptr;
jfieldID  gj_playerconfig_EnableTunnelRender        = nullptr;
jfieldID  gj_playerconfig_UserAgent                 = nullptr;
jfieldID  gj_playerconfig_NetworkRetryCount         = nullptr;


void JavaPlayerConfig::init(JNIEnv *env)
{
    if (gj_PlayerConfig_class == nullptr) {
        FindClass cls(env, playerConfigPath);
        gj_PlayerConfig_class                     = (jclass) env->NewGlobalRef(
                    cls.getClass());
        gj_PlayerConfig_Init                      = env->GetMethodID(gj_PlayerConfig_class,
                "<init>",
                "()V");
        gj_playerconfig_getCustomHeaders          = env->GetMethodID(gj_PlayerConfig_class,
                "getCustomHeaders",
                "()[Ljava/lang/String;");
        gj_playerconfig_setCustomHeaders          = env->GetMethodID(gj_PlayerConfig_class,
                "setCustomHeaders",
                "([Ljava/lang/String;)V");
        gj_playerconfig_HttpProxy                 = env->GetFieldID(gj_PlayerConfig_class,
                "mHttpProxy",
                "Ljava/lang/String;");
        gj_playerconfig_Referrer                  = env->GetFieldID(gj_PlayerConfig_class,
                "mReferrer",
                "Ljava/lang/String;");
        gj_playerconfig_UserAgent                 = env->GetFieldID(gj_PlayerConfig_class,
                "mUserAgent",
                "Ljava/lang/String;");
        gj_playerconfig_NetworkTimeout            = env->GetFieldID(gj_PlayerConfig_class,
                "mNetworkTimeout",
                "I");
        gj_playerconfig_MaxDelayTime              = env->GetFieldID(gj_PlayerConfig_class,
                "mMaxDelayTime",
                "I");
        gj_playerconfig_MaxBufferedPacketDuration = env->GetFieldID(gj_PlayerConfig_class,
                "mMaxBufferDuration",
                "I");
        gj_playerconfig_HighBufferLevel           = env->GetFieldID(gj_PlayerConfig_class,
                "mHighBufferDuration",
                "I");
        gj_playerconfig_FirstStartBufferLevel     = env->GetFieldID(gj_PlayerConfig_class,
                "mStartBufferDuration",
                "I");
        gj_playerconfig_MaxProbeSize              = env->GetFieldID(gj_PlayerConfig_class,
                "mMaxProbeSize",
                "I");
        gj_playerconfig_ClearFrameWhenStop        = env->GetFieldID(gj_PlayerConfig_class,
                "mClearFrameWhenStop",
                "Z");
        gj_playerconfig_EnableTunnelRender        = env->GetFieldID(gj_PlayerConfig_class,
                "mEnableVideoTunnelRender",
                "Z");
        gj_playerconfig_NetworkRetryCount         = env->GetFieldID(gj_PlayerConfig_class,
                "mNetworkRetryCount",
                "I");
    }
}

void JavaPlayerConfig::unInit(JNIEnv *pEnv)
{
    if (gj_PlayerConfig_class != nullptr) {
        pEnv->DeleteGlobalRef(gj_PlayerConfig_class);
        gj_PlayerConfig_class = nullptr;
    }
}


jobject JavaPlayerConfig::getJPlayerConfig(JNIEnv *mEnv, const MediaPlayerConfig *playerConfig)
{
    if (playerConfig == nullptr) {
        return nullptr;
    }

    jobject jPlayerConfig  = mEnv->NewObject(gj_PlayerConfig_class, gj_PlayerConfig_Init);
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_MaxBufferedPacketDuration,
                      playerConfig->maxBufferDuration);
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_FirstStartBufferLevel,
                      playerConfig->startBufferDuration);
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_HighBufferLevel,
                      playerConfig->highBufferDuration);
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_NetworkTimeout, playerConfig->networkTimeout);
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_MaxDelayTime, playerConfig->maxDelayTime);
    mEnv->SetBooleanField(jPlayerConfig, gj_playerconfig_ClearFrameWhenStop,
                          (jboolean) (playerConfig->bClearShowWhenStop));
    mEnv->SetBooleanField(jPlayerConfig, gj_playerconfig_EnableTunnelRender,
                          (jboolean) (playerConfig->bEnableTunnelRender));
    mEnv->SetIntField(jPlayerConfig, gj_playerconfig_NetworkRetryCount,
                      playerConfig->networkRetryCount);
    NewStringUTF tmpreferrer(mEnv, playerConfig->referer.c_str());
    jstring      referrer  = tmpreferrer.getString();
    mEnv->SetObjectField(jPlayerConfig, gj_playerconfig_Referrer, referrer);
    NewStringUTF tmphttpProxy(mEnv, playerConfig->httpProxy.c_str());
    jstring      httpProxy = tmphttpProxy.getString();
    mEnv->SetObjectField(jPlayerConfig, gj_playerconfig_HttpProxy, httpProxy);
    NewStringUTF tmpUserAgent(mEnv, playerConfig->userAgent.c_str());
    jstring      userAgent = tmpUserAgent.getString();
    mEnv->SetObjectField(jPlayerConfig, gj_playerconfig_UserAgent, userAgent);
    int headerLen = playerConfig->customHeaders.size();

    if (headerLen > 0) {
        jclass       stringClass  = mEnv->FindClass("java/lang/String");
        jobjectArray jheaderArray = mEnv->NewObjectArray(headerLen, stringClass, nullptr);

        for (int     i            = 0; i < headerLen; i++) {
            NewStringUTF newStringUTF(mEnv, playerConfig->customHeaders[i].c_str());
            jstring      header = newStringUTF.getString();
            mEnv->SetObjectArrayElement(jheaderArray, i, header);
        }

        mEnv->CallVoidMethod(jPlayerConfig, gj_playerconfig_setCustomHeaders, jheaderArray);
        mEnv->DeleteLocalRef(jheaderArray);
        mEnv->DeleteLocalRef(stringClass);
    }

    return jPlayerConfig;
}


MediaPlayerConfig JavaPlayerConfig::convertTo(JNIEnv *env, jobject playerConfig)
{
    MediaPlayerConfig config;

    if (playerConfig == nullptr) {
        return config;
    }

    GetObjectField    getHttpProxy(env, playerConfig, gj_playerconfig_HttpProxy);
    jstring           httpProxyStr              = static_cast<jstring>(getHttpProxy.getObjectField());
    GetObjectField    getRefer(env, playerConfig, gj_playerconfig_Referrer);
    jstring           referrerStr               = static_cast<jstring>(getRefer.getObjectField());
    GetObjectField    getUA(env, playerConfig, gj_playerconfig_UserAgent);
    jstring           userAgentStr              = static_cast<jstring>(getUA.getObjectField());
    jint              networkTimeout            = env->GetIntField(playerConfig,
            gj_playerconfig_NetworkTimeout);
    jint              maxDelayTime              = env->GetIntField(playerConfig,
            gj_playerconfig_MaxDelayTime);
    jint              maxBufferedPacketDuration = env->GetIntField(playerConfig,
            gj_playerconfig_MaxBufferedPacketDuration);
    jint              highBufferLevel           = env->GetIntField(playerConfig,
            gj_playerconfig_HighBufferLevel);
    jint              firstStartBufferLevel     = env->GetIntField(playerConfig,
            gj_playerconfig_FirstStartBufferLevel);
    jint              maxProbeSize              = env->GetIntField(playerConfig,
            gj_playerconfig_MaxProbeSize);
    jboolean          clearFrameWhenStop        = env->GetBooleanField(playerConfig,
            gj_playerconfig_ClearFrameWhenStop);
    jboolean          enableTunnelRender        = env->GetBooleanField(playerConfig,
            gj_playerconfig_EnableTunnelRender);
    jint              networkRetryCount         = env->GetIntField(playerConfig,
            gj_playerconfig_NetworkRetryCount);
    GetStringUTFChars tmpHttpProxy(env, httpProxyStr);
    char              *httpProxy                = tmpHttpProxy.getChars();
    GetStringUTFChars tmpreferrer(env, referrerStr);
    char              *referrer                 = tmpreferrer.getChars();
    GetStringUTFChars tmpUserAgent(env, userAgentStr);
    char              *userAgent                = tmpUserAgent.getChars();
    config.httpProxy              = (httpProxy == nullptr ? "" : std::string(httpProxy));
    config.referer                = (referrer == nullptr ? "" : std::string(referrer));
    config.userAgent              = (userAgent == nullptr ? "" : std::string(userAgent));
    config.startBufferDuration    = firstStartBufferLevel;
    config.highBufferDuration     = highBufferLevel;
    config.maxBufferDuration      = maxBufferedPacketDuration;
    config.maxDelayTime           = maxDelayTime;
    config.networkTimeout         = networkTimeout;
    config.bClearShowWhenStop     = clearFrameWhenStop;
    config.bEnableTunnelRender    = enableTunnelRender;
    config.networkRetryCount      = networkRetryCount;
    CallObjectMethod getHeaderMethod(env, playerConfig, gj_playerconfig_getCustomHeaders);
    jobjectArray     headersArray = (jobjectArray) getHeaderMethod.getValue();

    if (headersArray != nullptr) {
        int len = env->GetArrayLength(headersArray);

        if (len > 0) {
            for (int i = 0; i < len; i++) {
                jstring           header     = static_cast<jstring>(env->GetObjectArrayElement(
                                                   headersArray, i));
                GetStringUTFChars tmpHeader(env, header);
                char              *ch_header = tmpHeader.getChars();

                if (ch_header != nullptr) {
                    config.customHeaders.push_back(std::string(ch_header));
                }
            }
        }
    }

    return config;
}