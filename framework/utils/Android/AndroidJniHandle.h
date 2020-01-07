#ifndef ANDROIDJNIHANDLE_HH
#define ANDROIDJNIHANDLE_HH

#ifdef __ANDROID__

#include <jni.h>
#include "JniEnv.h"

namespace Cicada{ ; // namespace JniRegister

    template<class T>
    class AndroidJniHandle {
    public:
        explicit AndroidJniHandle(jobject t = nullptr) : obj_((T) t)
        {}

        virtual ~AndroidJniHandle()
        {
            release();
        }

        operator T() const
        {
            return obj_;
        }

        operator bool() const
        {
            return obj_ != nullptr;
        }

        AndroidJniHandle &operator=(jobject t)
        {
            if (obj_ != t) {
                release();
                obj_ = (T) t;
            }
            return *this;
        }

        AndroidJniHandle(const AndroidJniHandle &) = delete;

        AndroidJniHandle &operator=(const AndroidJniHandle &) = delete;

    protected:
        T obj_;

        void release()
        {
            if (obj_) {

                JniEnv jniEnv;
                JNIEnv* handle = jniEnv.getEnv();
                if (handle) {
                    handle->DeleteLocalRef(obj_);
                }
                obj_ = nullptr;
            }
        }
    };

    template<>
    class AndroidJniHandle<jstring> : public AndroidJniHandle<jobject> {
    public:
        explicit AndroidJniHandle<jstring>(jstring t)
                : AndroidJniHandle<jobject>((jobject) t)
        {}

        explicit AndroidJniHandle<jstring>(jobject t = nullptr)
                : AndroidJniHandle<jobject>(t)
        {}

        ~AndroidJniHandle()
        {
            release();
        }

        char const *GetStringUTFChars()
        {
            if (!ptr_) {
                if (obj_) {
                    JniEnv jniEnv;
                    JNIEnv* handle = jniEnv.getEnv();
                    if (handle) {
                        ptr_ = handle->GetStringUTFChars((jstring) obj_, nullptr);
                    }
                }
            }
            return ptr_;
        }

        jsize GetStringUTFLength()
        {
            if (lens_ == -1) {
                if (obj_) {

                    JniEnv jniEnv;
                    JNIEnv* handle = jniEnv.getEnv();
                    if (handle) {
                        lens_ = handle->GetStringUTFLength((jstring) obj_);
                    }
                }
            }
            return lens_;
        }

        operator jstring() const
        {
            return (jstring) obj_;
        }

        AndroidJniHandle<jstring> &operator=(jobject t)
        {
            if (obj_ != t) {
                release();
                obj_ = (jobject) t;
            }
            return *this;
        }

        AndroidJniHandle<jstring> &operator=(jstring t)
        {
            return operator=((jobject) t);
        }

        void release()
        {
            if (obj_) {

                JniEnv jniEnv;
                JNIEnv* handle = jniEnv.getEnv();
                if (handle) {
                    if (ptr_) {
                        handle->ReleaseStringUTFChars((jstring) obj_, ptr_);
                    }
                    handle->DeleteLocalRef(obj_);
                }
                ptr_ = nullptr;
                obj_ = nullptr;
                lens_ = -1;
            }
        }

    protected:
        char const *ptr_{nullptr};
        jsize lens_{-1};
    };
} // namespace Cicada
#endif //__ANDROID__
#endif // ANDROIDJNIHANDLE_HH
