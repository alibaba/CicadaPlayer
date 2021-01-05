//
// Created by lifujun on 2019/1/9.
//

#include "NewStringUTF.h"
#include "JniException.h"


NewStringUTF::NewStringUTF(JNIEnv *pEnv, const char *source)
{
    if (source == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        const char *errorKind = nullptr;
        checkUtfBytes(source, &errorKind);
        if (errorKind == nullptr) {
            mResult = pEnv->NewStringUTF(source);
        } else {
            mResult = pEnv->NewStringUTF("");
        }
        JniException::clearException(mEnv);
    }
}

NewStringUTF::~NewStringUTF()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jstring NewStringUTF::getString()
{
    return mResult;
}

char NewStringUTF::checkUtfBytes(const char *bytes, const char **errorKind)
{
    while (*bytes != '\0') {
        char utf8 = *(bytes++);
        // Switch on the high four bits.
        switch (utf8 >> 4) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
                // Bit pattern 0xxx. No need for any extra bytes.
                break;
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            case 0x0f:
                /*
                 * Bit pattern 10xx or 1111, which are illegal start bytes.
                 * Note: 1111 is valid for normal UTF-8, but not the
                 * modified UTF-8 used here.
                 */
                *errorKind = "start";
                return utf8;
            case 0x0e:
                // Bit pattern 1110, so there are two additional bytes.
                utf8 = *(bytes++);
                if ((utf8 & 0xc0) != 0x80) {
                    *errorKind = "continuation";
                    return utf8;
                }
                // Fall through to take care of the final byte.
            case 0x0c:
            case 0x0d:
                // Bit pattern 110x, so there is one additional byte.
                utf8 = *(bytes++);
                if ((utf8 & 0xc0) != 0x80) {
                    *errorKind = "continuation";
                    return utf8;
                }
                break;
        }
    }
    return 0;
}