#define LOG_TAG "mediaCodecDecoder"

#include <cstdlib>
#include "mediacodec_jni.h"
#include "mediaCodec.h"
#include <utils/frame_work_log.h>
#include <utils/Android/systemUtils.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>

using namespace std;
namespace Cicada {

    struct jfields {
        jclass media_codec_list_class, media_codec_class, media_format_class;
        jclass buffer_info_class, byte_buffer_class;
        jmethodID tostring;
        jmethodID init, get_codec_count, get_codec_info_at, is_encoder, get_capabilities_for_type, get_codec_infos;
        jmethodID is_feature_supported, get_video_capabilities, is_size_supported;
        jfieldID profile_levels_field, profile_field, level_field;
        jmethodID get_supported_types, get_name;
        jmethodID create_by_codec_name, configure, start, stop, flush, release;
        jmethodID get_output_format, set_onFrameRendered_listener, set_output_surface;
        jmethodID get_input_buffers, get_input_buffer;
        jmethodID get_output_buffers, get_output_buffer;
        jmethodID dequeue_input_buffer, dequeue_output_buffer, queue_input_buffer;
        jmethodID release_output_buffer;
        jmethodID create_video_format, create_audio_format;
        jmethodID set_integer, set_bytebuffer, get_integer;
        jmethodID buffer_info_ctor;
        jfieldID size_field, offset_field, pts_field, flags_field;
    };
    static struct jfields jfields;

    enum Types {
        METHOD, STATIC_METHOD, FIELD, STATIC_FIELD
    };

    struct classname {
        const char *name;
        ptrdiff_t offset;
    };

#define OFF(x) (ptrdiff_t)(&(((struct jfields *)0)->x))
    static const struct classname classes[] = {
        {"android/media/MediaCodecList",        OFF(media_codec_list_class)},
        {"android/media/MediaCodec",            OFF(media_codec_class)},
        {"android/media/MediaFormat",           OFF(media_format_class)},
        {"android/media/MediaCodec$BufferInfo", OFF(buffer_info_class)},
        {"java/nio/ByteBuffer",                 OFF(byte_buffer_class)},
        {nullptr, 0},
    };

    struct member {
        const char *name;
        const char *sig;
        const char *classpath;
        ptrdiff_t offset;
        int type;
        bool critical;
    };

    static const struct member members[] = {
        {"toString",                   "()Ljava/lang/String;",                                                             "java/lang/Object",                               OFF(tostring),                     METHOD,        true},

        {"getCodecCount",              "()I",                                                                              "android/media/MediaCodecList",                   OFF(get_codec_count),              STATIC_METHOD, true},
        {"<init>",                     "(I)V",                                                                             "android/media/MediaCodecList",                   OFF(init),                         METHOD,        false},
        {"getCodecInfoAt",             "(I)Landroid/media/MediaCodecInfo;",                                                "android/media/MediaCodecList",                   OFF(get_codec_info_at),            STATIC_METHOD, true},
        {"getCodecInfos",              "()[Landroid/media/MediaCodecInfo;",                                                "android/media/MediaCodecList",                   OFF(get_codec_infos),              METHOD,        false},
        {"isEncoder",                  "()Z",                                                                              "android/media/MediaCodecInfo",                   OFF(is_encoder),                   METHOD,        true},
        {"getSupportedTypes",          "()[Ljava/lang/String;",                                                            "android/media/MediaCodecInfo",                   OFF(get_supported_types),          METHOD,        true},
        {"getName",                    "()Ljava/lang/String;",                                                             "android/media/MediaCodecInfo",                   OFF(get_name),                     METHOD,        true},
        {"getCapabilitiesForType",     "(Ljava/lang/String;)Landroid/media/MediaCodecInfo$CodecCapabilities;",             "android/media/MediaCodecInfo",                   OFF(get_capabilities_for_type),    METHOD,        true},
        {"isFeatureSupported",         "(Ljava/lang/String;)Z",                                                            "android/media/MediaCodecInfo$CodecCapabilities", OFF(is_feature_supported),         METHOD,        false},
        {"getVideoCapabilities",       "()Landroid/media/MediaCodecInfo$VideoCapabilities;",                               "android/media/MediaCodecInfo$CodecCapabilities", OFF(get_video_capabilities),       METHOD,        false},
        {"isSizeSupported",            "(II)Z",                                                                            "android/media/MediaCodecInfo$VideoCapabilities", OFF(is_size_supported),            METHOD,        false},
        {"profileLevels",              "[Landroid/media/MediaCodecInfo$CodecProfileLevel;",                                "android/media/MediaCodecInfo$CodecCapabilities", OFF(profile_levels_field),         FIELD,         true},
        {"profile",                    "I",                                                                                "android/media/MediaCodecInfo$CodecProfileLevel", OFF(profile_field),                FIELD,         true},
        {"level",                      "I",                                                                                "android/media/MediaCodecInfo$CodecProfileLevel", OFF(level_field),                  FIELD,         true},

        {"createByCodecName",          "(Ljava/lang/String;)Landroid/media/MediaCodec;",                                   "android/media/MediaCodec",                       OFF(create_by_codec_name),         STATIC_METHOD, true},
        {"configure",                  "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V", "android/media/MediaCodec",                       OFF(configure),                    METHOD,        true},
        {"start",                      "()V",                                                                              "android/media/MediaCodec",                       OFF(start),                        METHOD,        true},
        {"stop",                       "()V",                                                                              "android/media/MediaCodec",                       OFF(stop),                         METHOD,        true},
        {"flush",                      "()V",                                                                              "android/media/MediaCodec",                       OFF(flush),                        METHOD,        true},
        {"release",                    "()V",                                                                              "android/media/MediaCodec",                       OFF(release),                      METHOD,        true},
        {"setOutputSurface",           "(Landroid/view/Surface;)V",                                                        "android/media/MediaCodec",                       OFF(set_output_surface),           METHOD,        false},
        {"setOnFrameRenderedListener", "(Landroid/media/MediaCodec$OnFrameRenderedListener;Landroid/os/Handler;)V",        "android/media/MediaCodec",                       OFF(set_onFrameRendered_listener), METHOD,        false},
        {"getOutputFormat",            "()Landroid/media/MediaFormat;",                                                    "android/media/MediaCodec",                       OFF(get_output_format),            METHOD,        true},
        {"getInputBuffers",            "()[Ljava/nio/ByteBuffer;",                                                         "android/media/MediaCodec",                       OFF(get_input_buffers),            METHOD,        false},
        {"getInputBuffer",             "(I)Ljava/nio/ByteBuffer;",                                                         "android/media/MediaCodec",                       OFF(get_input_buffer),             METHOD,        false},
        {"getOutputBuffers",           "()[Ljava/nio/ByteBuffer;",                                                         "android/media/MediaCodec",                       OFF(get_output_buffers),           METHOD,        false},
        {"getOutputBuffer",            "(I)Ljava/nio/ByteBuffer;",                                                         "android/media/MediaCodec",                       OFF(get_output_buffer),            METHOD,        false},
        {"dequeueInputBuffer",         "(J)I",                                                                             "android/media/MediaCodec",                       OFF(dequeue_input_buffer),         METHOD,        true},
        {"dequeueOutputBuffer",        "(Landroid/media/MediaCodec$BufferInfo;J)I",                                        "android/media/MediaCodec",                       OFF(dequeue_output_buffer),        METHOD,        true},
        {"queueInputBuffer",           "(IIIJI)V",                                                                         "android/media/MediaCodec",                       OFF(queue_input_buffer),           METHOD,        true},
        {"releaseOutputBuffer",        "(IZ)V",                                                                            "android/media/MediaCodec",                       OFF(release_output_buffer),        METHOD,        true},

        {"createVideoFormat",          "(Ljava/lang/String;II)Landroid/media/MediaFormat;",                                "android/media/MediaFormat",                      OFF(create_video_format),          STATIC_METHOD, true},
        {"createAudioFormat",          "(Ljava/lang/String;II)Landroid/media/MediaFormat;",                                "android/media/MediaFormat",                      OFF(create_audio_format),          STATIC_METHOD, true},
        {"setInteger",                 "(Ljava/lang/String;I)V",                                                           "android/media/MediaFormat",                      OFF(set_integer),                  METHOD,        true},
        {"getInteger",                 "(Ljava/lang/String;)I",                                                            "android/media/MediaFormat",                      OFF(get_integer),                  METHOD,        true},
        {"setByteBuffer",              "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V",                                       "android/media/MediaFormat",                      OFF(set_bytebuffer),               METHOD,        true},

        {"<init>",                     "()V",                                                                              "android/media/MediaCodec$BufferInfo",            OFF(buffer_info_ctor),             METHOD,        true},
        {"size",                       "I",                                                                                "android/media/MediaCodec$BufferInfo",            OFF(size_field),                   FIELD,         true},
        {"offset",                     "I",                                                                                "android/media/MediaCodec$BufferInfo",            OFF(offset_field),                 FIELD,         true},
        {"presentationTimeUs",         "J",                                                                                "android/media/MediaCodec$BufferInfo",            OFF(pts_field),                    FIELD,         true},
        {"flags",                      "I",                                                                                "android/media/MediaCodec$BufferInfo",            OFF(flags_field),                  FIELD,         true},
        {nullptr,                      nullptr,                                                                            nullptr, 0,                                                                          0,             false},
    };

    static inline jstring jni_new_string(JNIEnv *env, const char *psz_string)
    {
        return !JniException::clearException(env) ? env->NewStringUTF(psz_string) : nullptr;
    }

#define JNI_NEW_STRING(env, psz_string) jni_new_string(env, psz_string)

    static inline int get_integer(JNIEnv *env, jobject obj, const char *psz_name)
    {
        AndroidJniHandle<jstring> jname(JNI_NEW_STRING(env, psz_name));

        if (jname) {
            int i_ret = env->CallIntMethod(obj, jfields.get_integer, (jstring) jname);

            /* getInteger can throw nullptrPointerException (when fetching the
             * "channel-mask" property for example) */
            if (JniException::clearException(env)) {
                return MC_ERROR;
            }

            return i_ret;
        } else {
            return MC_ERROR;
        }
    }

#define GET_INTEGER(env, obj, name) get_integer(env, obj, name)

    static inline void set_integer(JNIEnv *env, jobject jobj, const char *psz_name,
                                   int i_value)
    {
        AndroidJniHandle<jstring> jname(JNI_NEW_STRING(env, psz_name));

        if (jname) {
            env->CallVoidMethod(jobj, jfields.set_integer, (jstring) jname, i_value);
        }
    }

#define SET_INTEGER(env, obj, name, value) set_integer(env, obj, name, value)

    /* Initialize all jni fields.
     * Done only one time during the first initialisation */
    static bool InitJNIFields(JNIEnv *env)
    {
        static int i_init_state = -1;
        bool ret;
        AndroidJniHandle<jclass> last_class;

        if (i_init_state != -1) {
            goto end;
        }

        i_init_state = 0;

        for (int i = 0; classes[i].name; i++) {
            AndroidJniHandle<jclass> clazz(env->FindClass(classes[i].name));

            if (JniException::clearException(env)) {
                AF_LOGE("Unable to find class %s", classes[i].name);
                goto end;
            }

            *(jclass *) ((uint8_t *) &jfields + classes[i].offset) =
                (jclass) env->NewGlobalRef(clazz);
        }

        for (int i = 0; members[i].name; i++) {
            if (i == 0 || strcmp(members[i].classpath, members[i - 1].classpath)) {
                last_class = env->FindClass(members[i].classpath);
            }

            if (JniException::clearException(env)) {
                AF_LOGE("Unable to find class %s", members[i].classpath);

                if (members[i].critical) {
                    goto end;
                } else {
                    continue;
                }
            }

            switch (members[i].type) {
                case METHOD:
                    *(jmethodID *) ((uint8_t *) &jfields + members[i].offset) =
                        env->GetMethodID(last_class, members[i].name, members[i].sig);
                    break;

                case STATIC_METHOD:
                    *(jmethodID *) ((uint8_t *) &jfields + members[i].offset) =
                        env->GetStaticMethodID(last_class, members[i].name, members[i].sig);
                    break;

                case FIELD:
                    *(jfieldID *) ((uint8_t *) &jfields + members[i].offset) =
                        env->GetFieldID(last_class, members[i].name, members[i].sig);
                    break;

                case STATIC_FIELD:
                    *(jfieldID *) ((uint8_t *) &jfields + members[i].offset) =
                        env->GetStaticFieldID(last_class, members[i].name, members[i].sig);
                    break;

                default:
                    break;
            }

            if (JniException::clearException(env)) {
                AF_LOGE("Unable to find the member %s in %s",
                        members[i].name, members[i].classpath);

                if (members[i].critical) {
                    goto end;
                }
            }
        }

        /* getInputBuffers and getOutputBuffers are deprecated if API >= 21
         * use getInputBuffer and getOutputBuffer instead. */
        if (jfields.get_input_buffer && jfields.get_output_buffer) {
            jfields.get_output_buffers = nullptr;
            jfields.get_input_buffers = nullptr;
        } else if (!jfields.get_output_buffers && !jfields.get_input_buffers) {
            AF_LOGE("Unable to find get Output/Input Buffer/Buffers");
            goto end;
        }

        i_init_state = 1;
end:
        ret = i_init_state == 1;

        if (!ret) {
            AF_LOGE("MediaCodec jni init failed");
        }

        return ret;
    }

    string mediaCodecGetName(const char *psz_mime, const mc_args &args,
                             size_t h264_profile, bool *p_adaptive)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("jni attach failed.");
            return string("");
        }

        if (!InitJNIFields((JNIEnv *) handle)) {
            return string("");
        }

        AndroidJniHandle<jstring> jmime(JNI_NEW_STRING((JNIEnv *) handle, psz_mime));

        if (!jmime) {
            return string("");
        }

        AndroidJniHandle<jobjectArray> codec_info_array;
        int num_codecs;
        int sdk_version = atoi(get_android_property("ro.build.version.sdk").c_str());
        AF_LOGI("device version %d", sdk_version);

        if (sdk_version >= 21) {
            AndroidJniHandle<jobject> codeclist(handle->NewObject(jfields.media_codec_list_class, jfields.init,
                                                REGULAR_CODECS));
            codec_info_array = handle->CallObjectMethod(codeclist,
                               jfields.get_codec_infos);
            num_codecs = handle->GetArrayLength(codec_info_array);
        } else {
            num_codecs = handle->CallStaticIntMethod(jfields.media_codec_list_class,
                         jfields.get_codec_count);
        }

        AF_LOGI("mediacodec codec info list inited.");

        for (int i = 0; i < num_codecs; i++) {
            AndroidJniHandle<jobject> info;

            if (sdk_version >= 21) {
                info = handle->GetObjectArrayElement(codec_info_array, i);
            } else {
                info = handle->CallStaticObjectMethod(jfields.media_codec_list_class,
                                                      jfields.get_codec_info_at, i);
            }

            AndroidJniHandle<jstring> name(handle->CallObjectMethod(info, jfields.get_name));

            if (OMXCodec_IsBlacklisted(name.GetStringUTFChars(), name.GetStringUTFLength())) {
                continue;
            }

            if (handle->CallBooleanMethod(info, jfields.is_encoder)) {
                continue;
            }

            AndroidJniHandle<jobject> codec_capabilities(handle->CallObjectMethod(info,
                    jfields.get_capabilities_for_type,
                    (jstring) jmime));

            if (JniException::clearException(handle)) {
                AF_LOGW("Exception occurred in MediaCodecInfo.getCapabilitiesForType");
                continue;
            }

            AndroidJniHandle<jobjectArray> profile_levels;
            int profile_levels_len = 0;
            bool b_adaptive = false;

            if (codec_capabilities) {
                profile_levels = (jobjectArray) handle->GetObjectField(codec_capabilities,
                                 jfields.profile_levels_field);

                if (profile_levels) {
                    profile_levels_len = handle->GetArrayLength(profile_levels);
                }

                if (jfields.is_feature_supported) {
                    AndroidJniHandle<jstring> jfeature(JNI_NEW_STRING((JNIEnv *) handle, "adaptive-playback"));
                    b_adaptive = handle->CallBooleanMethod(codec_capabilities,
                                                           jfields.is_feature_supported,
                                                           (jstring) jfeature);
                    JniException::clearException(handle);
                }

                if (jfields.get_video_capabilities) {
                    AndroidJniHandle<jobject> video_capabilities(handle->CallObjectMethod(codec_capabilities,
                            jfields.get_video_capabilities));

                    if (jfields.is_size_supported) {
                        bool ret = handle->CallBooleanMethod(video_capabilities,
                                                             jfields.is_size_supported,
                                                             args.video.width,
                                                             args.video.height);

                        if (!ret && args.video.width < args.video.height) {
                            AF_LOGW("width %d < height %d swap and tryAgain.",
                                    args.video.width, args.video.height);
                            ret = handle->CallBooleanMethod(video_capabilities,
                                                            jfields.is_size_supported,
                                                            args.video.height,
                                                            args.video.width);
                        }

                        AF_LOGI("psz_mime %s, width %d, height %d, is_size_supported ? %d",
                                psz_mime, args.video.width, args.video.height, ret);

                        if (!ret) {
                            continue;
                        }
                    }
                }
            }

            AF_LOGI("Number of profile levels: %d", profile_levels_len);
            AndroidJniHandle<jobjectArray> types(handle->CallObjectMethod(info, jfields.get_supported_types));
            int num_types = handle->GetArrayLength(types);
            bool found = false;

            for (int j = 0; j < num_types && !found; j++) {
                AndroidJniHandle<jstring> type((jstring) handle->GetObjectArrayElement(types, j));

                if (!strncmp(psz_mime, type.GetStringUTFChars(), type.GetStringUTFLength())) {
                    /* The mime type is matching for this component. We
                       now check if the capabilities of the codec is
                       matching the video format. */
                    if (h264_profile) {
                        /* This decoder doesn't expose its profiles and is high
                         * profile capable */
                        if (!strncmp(name.GetStringUTFChars(), "OMX.LUMEVideoDecoder", __MIN(20, name.GetStringUTFLength()))) {
                            found = true;
                        }

                        for (int i = 0; i < profile_levels_len && !found; ++i) {
                            AndroidJniHandle<jobject> profile_level(handle->GetObjectArrayElement(profile_levels, i));
                            int omx_profile = handle->GetIntField(profile_level,
                                                                  jfields.profile_field);
                            size_t codec_profile = convert_omx_to_profile_idc(
                                                       selectprofiletype(omx_profile));

                            if (codec_profile != h264_profile) {
                                continue;
                            }

                            /* Some encoders set the level too high, thus we ignore it for the moment.
                               We could try to guess the actual profile based on the resolution. */
                            found = true;
                        }
                    } else {
                        found = true;
                    }
                }
            }

            if (found) {
                AF_LOGI("using %s", name.GetStringUTFChars());
                *p_adaptive = b_adaptive;
                return string(name.GetStringUTFChars(), name.GetStringUTFLength());
            }
        }

        AF_LOGI("not found mediacodec.");
        return string("");
    }

    int MediaCodec_JNI::init(const char *mime, int category, jobject surface)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        if (!InitJNIFields((JNIEnv *) handle)) {
            AF_LOGE("Android MediaCodec init jni fields failed!");
            return MC_ERROR;
        }

        mSurface = surface;
        psz_mime = mime;
        category_codec = category;
        return 0;
    }

    int MediaCodec_JNI::start()
    {
        int i_ret = MC_ERROR;
        AndroidJniHandle<jobject> jinput_buffers;
        AndroidJniHandle<jobject> joutput_buffers;
        AndroidJniHandle<jobject> jbuffer_info;
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return -1;
        }

        handle->CallVoidMethod(codec, jfields.start);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception occurred in MediaCodec.start");
            goto error;
        }

        b_started = true;

        if (jfields.get_input_buffers && jfields.get_output_buffers) {
            jinput_buffers = handle->CallObjectMethod(codec,
                             jfields.get_input_buffers);

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.getInputBuffers");
                goto error;
            }

            input_buffers = (jobjectArray) handle->NewGlobalRef(jinput_buffers);
            joutput_buffers = handle->CallObjectMethod(codec,
                              jfields.get_output_buffers);

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.getOutputBuffers");
                goto error;
            }

            output_buffers = (jobjectArray) handle->NewGlobalRef(joutput_buffers);
        }

        jbuffer_info = handle->NewObject(jfields.buffer_info_class,
                                         jfields.buffer_info_ctor);
        buffer_info = handle->NewGlobalRef(jbuffer_info);
        i_ret = 0;
        AF_LOGI("MediaCodec via JNI opened");
error:

        if (i_ret != 0) {
            stop();
        }

        return i_ret;
    }

    int MediaCodec_JNI::stop()
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        if (buffer_info) {
            handle->DeleteGlobalRef(buffer_info);
            buffer_info = nullptr;
        }

        if (input_buffers) {
            handle->DeleteGlobalRef(input_buffers);
            input_buffers = nullptr;
        }

        if (output_buffers) {
            handle->DeleteGlobalRef(output_buffers);
            output_buffers = nullptr;
        }

        if (codec) {
            if (b_started) {
                handle->CallVoidMethod(codec, jfields.stop);

                if (JniException::clearException(handle)) {
                    AF_LOGE("Exception in MediaCodec.stop");
                }

                b_started = false;
            }
        }

        return 0;
    }

    int MediaCodec_JNI::flush()
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        handle->CallVoidMethod(codec, jfields.flush);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception occurred in MediaCodec.flush");
            return MC_ERROR;
        }

        return 0;
    }

    int MediaCodec_JNI::dequeue_in(int64_t timeout)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        int index;
        index = handle->CallIntMethod(codec,
                                      jfields.dequeue_input_buffer, timeout);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception occurred in MediaCodec.dequeueInputBuffer");
            return MC_ERROR;
        }

        if (index >= 0) {
            return index;
        } else {
            return MC_INFO_TRYAGAIN;
        }
    }

    int MediaCodec_JNI::queue_in(int index, const void *p_buf, size_t size, int64_t pts, bool config)
    {
        if (index < 0) {
            return MC_ERROR;
        }

        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        AndroidJniHandle<jobject> j_mc_buf;

        if (jfields.get_input_buffers) {
            j_mc_buf = handle->GetObjectArrayElement(input_buffers, index);
        } else {
            j_mc_buf = handle->CallObjectMethod(codec,
                                                jfields.get_input_buffer, index);

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.getInputBuffer");
                return MC_ERROR;
            }
        }

        jsize j_mc_size = handle->GetDirectBufferCapacity(j_mc_buf);
        auto *p_mc_buf = (uint8_t *) handle->GetDirectBufferAddress(j_mc_buf);

        if (j_mc_size < 0) {
            AF_LOGE("Java buffer has invalid size");
            return MC_ERROR;
        }

        if ((size_t) j_mc_size > size) {
            j_mc_size = size;
        }

        if (p_buf != nullptr) {
            memcpy(p_mc_buf, p_buf, j_mc_size);
        }

        jint jflags = 0;

        if (config) {
            jflags |= BUFFER_FLAG_CODEC_CONFIG;
        }

        if (p_buf == nullptr) {
            jflags |= BUFFER_FLAG_END_OF_STREAM;
        }

        handle->CallVoidMethod(codec, jfields.queue_input_buffer,
                               index, 0, j_mc_size, pts, jflags);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception in MediaCodec.queueInputBuffer");
            return MC_ERROR;
        }

        return 0;
    }

    int MediaCodec_JNI::dequeue_out(int64_t timeout)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        int index = handle->CallIntMethod(codec,
                                          jfields.dequeue_output_buffer,
                                          buffer_info, timeout);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception in MediaCodec.dequeueOutputBuffer");
            return MC_ERROR;
        }

        if (index >= 0) {
            return index;
        } else if (index == INFO_OUTPUT_FORMAT_CHANGED) {
            return MC_INFO_OUTPUT_FORMAT_CHANGED;
        } else if (index == INFO_OUTPUT_BUFFERS_CHANGED) {
            return MC_INFO_OUTPUT_BUFFERS_CHANGED;
        } else {
            return MC_INFO_TRYAGAIN;
        }
    }

    int MediaCodec_JNI::get_out(int index, mc_out *out, bool readBuffer)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        if (index >= 0) {
            out->type = MC_OUT_TYPE_BUF;
            out->buf.index = index;
            out->buf.pts = handle->GetLongField(buffer_info,
                                                jfields.pts_field);
            int flags = handle->GetIntField(buffer_info,
                                            jfields.flags_field);

            if ((flags & BUFFER_FLAG_END_OF_STREAM) != 0) {
                out->b_eos = true;
            } else {
                out->b_eos = false;
            }

            AndroidJniHandle<jobject> buf;
            uint8_t *ptr = NULL;
            int offset = 0;

            if (readBuffer) {
                if (jfields.get_output_buffers) {
                    buf = handle->GetObjectArrayElement(output_buffers, index);
                } else {
                    buf = handle->CallObjectMethod(codec,
                                                   jfields.get_output_buffer,
                                                   index);

                    if (JniException::clearException(handle)) {
                        AF_LOGE("Exception in MediaCodec.getOutputBuffer");
                        return MC_ERROR;
                    }
                }

                //jsize buf_size = (*env)->GetDirectBufferCapacity(env, buf);
                /* buf can be NULL in case of EOS */
                if (buf) {
                    ptr = (uint8_t *) handle->GetDirectBufferAddress(buf);
                    offset = handle->GetIntField(buffer_info,
                                                 jfields.offset_field);
                }

                out->buf.p_ptr = ptr + offset;
                out->buf.size = handle->GetIntField(buffer_info, jfields.size_field);
            } else {
                out->buf.p_ptr = nullptr;
                out->buf.size = 0;
            }

            return 1;
        } else if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
            AndroidJniHandle<jobject> format(handle->CallObjectMethod(codec,
                                             jfields.get_output_format));

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.getOutputFormat");
                return MC_ERROR;
            }

            AndroidJniHandle<jstring> format_string(handle->CallObjectMethod(format, jfields.tostring));
            AF_LOGI("output format changed: %s", format_string.GetStringUTFChars());
            out->type = MC_OUT_TYPE_CONF;
            out->b_eos = false;

            if (category_codec == CATEGORY_VIDEO) {
                out->conf.video.width = GET_INTEGER((JNIEnv *) handle, format, "width");
                out->conf.video.height = GET_INTEGER((JNIEnv *) handle, format, "height");
                out->conf.video.stride = GET_INTEGER((JNIEnv *) handle, format, "stride");
                out->conf.video.slice_height = GET_INTEGER((JNIEnv *) handle, format, "slice-height");
                out->conf.video.pixel_format = GET_INTEGER((JNIEnv *) handle, format, "color-format");
                out->conf.video.crop_left = GET_INTEGER((JNIEnv *) handle, format, "crop-left");
                out->conf.video.crop_top = GET_INTEGER((JNIEnv *) handle, format, "crop-top");
                out->conf.video.crop_right = GET_INTEGER((JNIEnv *) handle, format, "crop-right");
                out->conf.video.crop_bottom = GET_INTEGER((JNIEnv *) handle, format, "crop-bottom");
                AF_LOGI("width %d height %d stride %d slice_height %d crop right %d",
                        out->conf.video.width, out->conf.video.height, out->conf.video.stride,
                        out->conf.video.slice_height, out->conf.video.crop_right);
            } else {
                out->conf.audio.channel_count = GET_INTEGER((JNIEnv *) handle, format, "channel-count");
                out->conf.audio.channel_mask = GET_INTEGER((JNIEnv *) handle, format, "channel-mask");
                out->conf.audio.sample_rate = GET_INTEGER((JNIEnv *) handle, format, "sample-rate");
            }

            return 1;
        } else if (index == MC_INFO_OUTPUT_BUFFERS_CHANGED) {
            AF_LOGI("output buffers changed");

            if (!jfields.get_output_buffers) {
                return 0;
            }

            handle->DeleteGlobalRef(output_buffers);
            output_buffers = nullptr;
            AndroidJniHandle<jobject> joutput_buffers(handle->CallObjectMethod(codec,
                    jfields.get_output_buffers));

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.getOutputBuffer");
                return MC_ERROR;
            }

            output_buffers = (jobjectArray) handle->NewGlobalRef(joutput_buffers);
        }

        return 0;
    }

    int MediaCodec_JNI::configure(size_t i_h264_profile, const mc_args &args)
    {
        bool b_adaptive;
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();
        std::string psz_name = mediaCodecGetName(psz_mime, args, i_h264_profile, &b_adaptive);
        AF_LOGI("android decode name %s, b_adaptive %d", psz_name.c_str(), b_adaptive);

        if (psz_name.empty()) {
            AF_LOGE("can not get mediacodec name.");
            return -11;
        }

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return -12;
        }

        AndroidJniHandle<jstring> jmime(JNI_NEW_STRING((JNIEnv *) handle, psz_mime));
        AndroidJniHandle<jstring> jcodec_name(JNI_NEW_STRING((JNIEnv *) handle, psz_name.c_str()));

        if (!jmime || !jcodec_name) {
            AF_LOGE("find jcodec name failed.");
            return -13;
        }

        /* This method doesn't handle errors nicely, it crashes if the codec isn't
         * found.  (The same goes for createDecoderByType.) This is fixed in latest
         * AOSP and in 4.2, but not in 4.1 devices. */
        AndroidJniHandle<jobject> jcodec(handle->CallStaticObjectMethod(jfields.media_codec_class,
                                         jfields.create_by_codec_name,
                                         (jstring) jcodec_name));

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception occurred in MediaCodec.createByCodecName %s", psz_name.c_str());
            return -14;
        }

        codec = handle->NewGlobalRef(jcodec);
        AndroidJniHandle<jobject> jformat;

        if (category_codec == CATEGORY_VIDEO) {
//        assert(args->video.i_angle == 0 || api->b_support_rotation);
            jformat = handle->CallStaticObjectMethod(
                          jfields.media_format_class,
                          jfields.create_video_format,
                          (jstring) jmime,
                          args.video.width,
                          args.video.height);

            if (args.video.angle != 0) {
                SET_INTEGER((JNIEnv *) handle, jformat, "rotation-degrees", args.video.angle);
            }
        } else {
            jformat = handle->CallStaticObjectMethod(jfields.media_format_class,
                      jfields.create_audio_format,
                      (jstring) jmime,
                      args.audio.sample_rate,
                      args.audio.channel_count);
        }

        /* No limits for input size */
        SET_INTEGER((JNIEnv *) handle, jformat, "max-input-size", 0);
        handle->CallVoidMethod(codec, jfields.configure,
                               (jobject) jformat, mSurface, NULL, 0);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception occurred in MediaCodec.configure");
            return -15;
        }

        return 0;
    }


    void MediaCodec_JNI::unInit()
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return;
        }

        if (codec) {
            handle->CallVoidMethod(codec, jfields.release);

            if (JniException::clearException(handle)) {
                AF_LOGE("Exception in MediaCodec.release");
            }

            handle->DeleteGlobalRef(codec);
            codec = NULL;
        }
    }

    int MediaCodec_JNI::setOutputSurface(jobject surface)
    {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        handle->CallVoidMethod(codec, jfields.set_output_surface, surface);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception in MediaCodec.setOutputSurface");
            return MC_ERROR;
        }

        return 0;
    }

    int MediaCodec_JNI::release_out(int index, bool render)
    {
        if (index < 0) {
            return MC_ERROR;
        }

        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("env is nullptr.");
            return MC_ERROR;
        }

        handle->CallVoidMethod(codec, jfields.release_output_buffer,
                               index, render);

        if (JniException::clearException(handle)) {
            AF_LOGE("Exception in MediaCodec.releaseOutputBuffer");
            return MC_ERROR;
        }

        return 0;
    }
}
