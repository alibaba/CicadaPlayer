#ifndef CICADA_ERROR_EVENT_DEF_H
#define CICADA_ERROR_EVENT_DEF_H

namespace Cicada
{

enum CicadaErrorType
{
    /**
     * 成功
     */
    CICADA_COMMON_RETURN_SUCCESS =                                    0,

    /**
     * 失败
     */
    CICADA_COMMON_RETURN_FAILED =                                    -1,

    /**
     * 参数错误
     */
    CICADA_COMMON_INVALID_PARAM =                                    -2,

    /**
     * 未知错误
     */
    CICADA_COMMON_UNKNOWN_ERROR_CODE =                               -3,

    /**
     * 状态错误
     */
    CICADA_COMMON_INVALID_STATE =                                    -4,

    /**
     * MDF起始错误段
     */
    CICADA_FRAMEWORK_ERROR_START =                                   -10000000,

    /**
     * 线程退出异常
     */
    CICADA_FRAMEWORK_ERROR_THREAD_EXIT =                             -10000001,

    /**
     * 分发消息失败
     */
    CICADA_FRAMEWORK_DISPATCH_MSG_FAILED =                           -10000002,

    /**
     * 标记同步消息
     */
    CICADA_FRAMEWORK_IS_SYNC_MSG =                                   -10000003,

    /**
     * 发送同步消息超时
     */
    CICADA_FRAMEWORK_SEND_SYNC_MSG_TIME_OUT =                        -10000004,

    /**
     * 消息队列满
     */
    CICADA_FRAMEWORK_ERROR_MSG_QUEUE_FULL =                          -10000005,

    /**
     * 该服务已存在
     */
    CICADA_FRAMEWORK_ERROR_SERVICE_IS_ALREADY_EXIST =                -10000006,

    /**
     * 组件切换时状态为空
     */
    CICADA_FRAMEWORK_ERROR_SERVICE_CHANGE_STATE_NULL =               -10000007,

    /**
     * 组件切换PREPARED状态失败
     */
    CICADA_FRAMEWORK_ERROR_SERVICE_CHANGE_PREPARED_STATE_FAILED =    -10000008,

    /**
     * 组件切换PLAYING状态失败
     */
    CICADA_FRAMEWORK_ERROR_SERVICE_CHANGE_PLAYING_STATE_FAILED =     -10000009,

    /**
     * 组件切换INITED状态失败
     */
    CICADA_FRAMEWORK_ERROR_SERVICE_CHANGE_INITED_STATE_FAILED =      -10000010,

    /**
     * 封装模块起始错误段
     */
    CICADA_FRAMEWORK_MUXER_ERROR_START =                             -10001000,

    /**
     * 创建封装上下文失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_CREAT_OUTPUT =                      -10001001,

    /**
     * 创建封装音频上下文失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_CREATE_ACTX =                       -10001002,

    /**
     * 创建封装流失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_CREATE_STREAM =                     -10001003,

    /**
     * 创建封装视频上下文失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_CREATE_VCTX =                       -10001004,

    /**
     * 写入封装数据包失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_INPUT_PACKET =                      -10001005,

    /**
     * 设置音视频流参数异常
     */
    CICADA_FRAMEWORK_MUXER_ERROR_VIDEO_AUDIO_UNSET =                 -10001006,

    /**
     * 封装服务状态异常
     */
    CICADA_FRAMEWORK_MUXER_ERROR_STATE =                             -10001007,

    /**
     * 写入封装文件头失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_WRITE_HEADER =                      -10001008,

    /**
     * 写入封装文件尾失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_WRITE_TRAILER =                     -10001009,

    /**
     * 函数传入参数错误，如传入空的输入路径名、输出路径名、音频采样率、音频通道数、分辨率等
     */
    CICADA_FRAMEWORK_MUXER_ERROR_INVALIDATE_PARAM =                  -10001010,

    /**
     * 无法找到封装指定的编码器类型
     */
    CICADA_FRAMEWORK_MUXER_ERROR_FIND_ENCODER =                      -10001011,

    /**
     * 创建封装流失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_NEW_STREAM =                        -10001012,

    /**
     * 创建封装上下文失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_NEW_ALLOC_OUTPUT_CONTEXT =          -10001013,

    /**
     * 创建封装IO上下文失败
     */
    CICADA_FRAMEWORK_MUXER_ERROR_AVIO_OPEN =                         -10001014,

    /**
     * 解封装起始错误段
     */
    CICADA_FRAMEWORK_DEMUXER_ERROR_START =                           -10002000,

    /**
     * 文件解析失败
     */
    CICADA_FRAMEWORK_DEMUXER_ERROR_INPUT_FILE =                      -10002001,

    /**
     * 文件重复解析
     */
    CICADA_FRAMEWORK_DEMUXER_INIT_MULTI_TIMES =                      -10002002,

    /**
     * 文件打开失败
     */
    CICADA_FRAMEWORK_DEMUXER_OPEN_FILE_FAILED =                      -10002003,

    /**
     * 文件流未找到
     */
    CICADA_FRAMEWORK_DEMUXER_FIND_STREAM_INFO_FAILED =               -10002004,

    /**
     * 文件解析结束出错
     */
    CICADA_FRAMEWORK_DEMUXER_ERROR_IN_END =                          -10002005,

    /**
     * 视频编码组件起始错误段
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_START =                     -10003000,

    /**
     * 创建视频编码器失败，参数不支持
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_CREATE_ENCODER_FAILED =           -10003001,

    /**
     * 创建视频编码器失败，没有符合该编码场景要求的编码器
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_WIDTHOUT_MATCH_ENCODER =          -10003002,

    /**
     * 视频编码组件没有设置输出数据的接收方，无效工作
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_WITHOUT_WORK =              -10003003,

    /**
     * 视频编码器被打断，运行中报错
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_INTERRUPT =                 -10003004,

    /**
     * 系统版本限制导致视频硬编码器创建失败
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_ANDROID_API_LEVEL =         -10003005,

    /**
     * 视频编码器启动失败
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_STATE =                     -10003006,

    /**
     * 视频帧输入数据不合法
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_INPUT =                     -10003007,

    /**
     * 视频编码器内部缓存已满，需要等待
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_NO_BUFFER_AVAILABLE =       -10003008,

    /**
     * iOS视频硬编码session报错
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_SESSION_BORKEN =            -10003009,

    /**
     * 输入数据的pts非法，不符合编码要求
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_INVALID_PTS =               -10003010,

    /**
     * 视频编码器被暂停，等待唤醒
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_NOT_ACTIVE =                -10003011,

    /**
     * 视频编码器被暂停，等待唤醒
     */
    CICADA_FRAMEWORK_VIDEO_ENCODER_ERROR_CB =                        -10003012,

    // video decoder
    /**
     * 视频解码起始段
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_START =                     -10004000,
    /**
     * 无SPS
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_SPS_PPS_NULL =                    -10004001,
    /**
     * 创建H264解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_CREATE_H264_PARAM_SET_FAILED =    -10004002,
    /**
     * 创建HEVC解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_CREATE_HEVC_PARAM_SET_FAILED =    -10004003,
    /**
     * 创建解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_CREATE_DECODER_FAILED =           -10004004,
    /**
     * 解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_STATE =                     -10004005,
    /**
     * 创建解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_INPUT =                     -10004006,
    /**
     * 解码没有足够缓存
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_NO_BUFFER_AVAILABLE =       -10004007,
    /**
     * 解码被中断
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_INTERRUPT =                 -10004008,
    /**
     * 解码SPS失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_DECODE_SPS =                -10004009,
    /**
     * 解码资源失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_RESOURCE_PREEMPTED =        -10004010,
    /**
     * 解码失败
     */
    CICADA_FRAMEWORK_VIDEO_DECODER_ERROR_DECODE_PACKET =             -10004011,

    /**
     * 软解不支持设置OES格式输出
     */
    CICADA_VDEC_ERROR_FFMPEG_NOT_SUPPORT_OES                       = -10004101,
    /**
     * 软解找不到codec对应的解码器
     */
    CICADA_VDEC_ERROR_FFMPEG_NOT_FIND_DECODER                      = -10004102,
    /**
     * 软解解码找不到上下文
     */
    CICADA_VDEC_ERROR_FFMPEG_DECODE_PACKET_INVALID                 = -10004103,
    /**
     * 软解解码创建上下文失败
     */
    CICADA_VDEC_ERROR_FFMPEG_NO_CONTEXT                            = -10004104,
    /**
     * 软解内部打开失败
     */
    CICADA_VDEC_ERROR_FFMPEG_OPEN_CODEC                            = -10004105,
    /**
     * 软解解码一帧失败
     */
    CICADA_VDEC_ERROR_FFMPEG_DEC_VIDEO                             = -10004106,
    /**
     * 软解未知的输出格式
     */
    CICADA_VDEC_ERROR_FFMPEG_UNKNOWN_FORMAT                        = -10004107,
    /**
     * 软解未知的输入帧类型
     */
    CICADA_VDEC_ERROR_FFMPEG_TYPE_INVALID                          = -10004108,
    /**
     * 软解Flush失败
     */
    CICADA_VDEC_ERROR_FFMPEG_FLUSH_FAILED                          = -10004109,
    /**
     * Android硬解不支持，因为命中黑名单
     */
    CICADA_VDEC_ERROR_API18_BLACKLIST                              = -10004201,
    /**
     * Android硬解不支持，因为编码类型
     */
    CICADA_VDEC_ERROR_API18_CODEC_NOT_SUPPORT                      = -10004202,
    /**
     * Android硬解创建SurfaceTexture失败
     */
    CICADA_VDEC_ERROR_API18_CREATE_SURFACE_FAILED                  = -10004203,
    /**
     * Android硬解mediacodec初始化失败
     */
    CICADA_VDEC_ERROR_API18_DECODER_INIT_ERROR                     = -10004204,
    /**
     * Android硬解mediacodec configure失败
     */
    CICADA_VDEC_ERROR_API18_DECODER_CFG_ERROR                      = -10004205,
    /**
     * Android硬解未知的输入帧类型
     */
    CICADA_VDEC_ERROR_API18_PACKET_TYPE_INVALID                    = -10004206,
    /**
     * Android硬解mediacodec queueInput失败
     */
    CICADA_VDEC_ERROR_API18_SPS_DEQUEUE_IN                         = -10004207,
    /**
     * Android硬解mediacodec start失败
     */
    CICADA_VDEC_ERROR_API18_START                                  = -10004208,
    /**
     * Android硬解mediacodec dequeueInput失败
     */
    CICADA_VDEC_ERROR_API18_FLUSH_DEQUEUE_IN                       = -10004209,
    /**
     * Android硬解mediacodec dequeueInput失败
     */
    CICADA_VDEC_ERROR_API18_SLICE_DEQUEUE_IN                       = -10004210,
    /**
     * Android硬解mediacodec dequeueOut失败
     */
    CICADA_VDEC_ERROR_API18_SLICE_DEQUEUE_OUT                      = -10004211,
    /**
     * Android硬解mediacodec queueIn失败
     */
    CICADA_VDEC_ERROR_API18_SLICE_QUEUE_IN                         = -10004212,
    /**
     * Android硬解mediacodec queueIn失败
     */
    CICADA_VDEC_ERROR_API18_SLICE_QUEUE_IN_INFO                    = -10004213,
    /**
     * Android硬解mediacodec queueIn失败
     */
    CICADA_VDEC_ERROR_API18_EOS_QUEUE_IN                           = -10004214,
    /**
     * Android硬解mediacodec flush失败
     */
    CICADA_VDEC_ERROR_API18_FLUSH                                  = -10004215,
    /**
     * Android硬解flush超时
     */
    CICADA_VDEC_ERROR_API18_FLUSHCODEC                             = -10004216,
    /**
     * Android硬解flush中断
     */
    CICADA_VDEC_ERROR_API18_FLUSH_INTERRUPT                        = -10004217,
    /**
     * iOS硬解码 版本太低导致hevc失败
     */
    CICADA_VDEC_ERROR_IOS_HEVC_VERSION_LOW                         = -10004301,
    /**
     * iOS硬解码 硬件不支持
     */
    CICADA_VDEC_ERROR_IOS_HEVC_HW_NOT_SUPPORT                      = -10004302,
    /**
     * iOS硬解码 编译平台错误
     */
    CICADA_VDEC_ERROR_IOS_PLATFORM_ERROR                           = -10004303,
    /**
     * iOS硬解码 不支持解码类型
     */
    CICADA_VDEC_ERROR_IOS_CODEC_NOT_SUPPORT                        = -10004304,
    /**
     * iOS硬解码 输入帧有误
     */
    CICADA_VDEC_ERROR_IOS_INVALID_PACKET                           = -10004305,
    /**
     * iOS硬解码 输入帧解析失败
     */
    CICADA_VDEC_ERROR_IOS_INVALID_PACKET2                          = -10004306,
    /**
     * iOS硬解码 编译平台错误
     */
    CICADA_VDEC_ERROR_IOS_PLATFORM_ERROR2                          = -10004308,
    /**
     * iOS硬解码 createSession失败
     */
    CICADA_VDEC_ERROR_IOS_DECODER_CREATE                           = -10004309,
    /**
     * iOS硬解码 createBuffer失败
     */
    CICADA_VDEC_ERROR_IOS_CREATE_BUFFER                            = -10004310,
    /**
     * iOS硬解码 createSampleBuffer失败
     */
    CICADA_VDEC_ERROR_IOS_CREATE_SAMPLE                            = -10004311,
    /**
     * iOS硬解码 解码失败kVTInvalidSessionErr
     */
    CICADA_VDEC_ERROR_IOS_RESOURCE_INVALID_ERROR                   = -10004312,
    /**
     * iOS硬解码 解码失败
     */
    CICADA_VDEC_ERROR_IOS_DECODE_ERROR                             = -10004313,
    /**
     * iOS硬解码 解码回调错误
     */
    CICADA_VDEC_ERROR_IOS_FETCHFRAME_ERROR                         = -10004314,
    /**
     * iOS硬解码 创建format失败
     */
    CICADA_VDEC_ERROR_IOS_CREATE_FORMAT                            = -10004315,

    /**
     * 音频编码组件起始错误段
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_ERROR_START =                     -10005000,

    /**
     * 创建音频编码器失败，参数不支持
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_INIT_FAILED =                     -10005001,

    /**
     * 音频编码组件状态不符，出现了重复创建或者重复销毁
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_ERROR_STATE =                     -10005002,

    /**
     * 音频帧输入数据不合法
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_ERROR_INPUT =                     -10005003,

    /**
     * 音频编码器被打断，无法继续工作
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_ERROR_INTERRUPT =                 -10005004,

    /**
     * 音频编码组件没有设置数据输出的接收端，无意义工作
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_ERROR_WITHOUT_WORK =              -10005005,

    /**
     * 没有符合编码场景要求的编码器
     */
    CICADA_FRAMEWORK_AUDIO_ENCODER_WIDTHOUT_MATCH_ENCODER =          -10005006,

    /**
     * 音频解码组件起始错误段
     */
    CICADA_FRAMEWORK_AUDIO_DECODER_ERROR_START =                     -10006000,

    /**
     * 创建音频解码器失败，参数不支持
     */
    CICADA_FRAMEWORK_AUDIO_DECODER_CREATE_DECODER_FAILED =           -10006001,

    /**
     * 音频解码组件状态不符，出现了重复创建或者重复销毁
     */
    CICADA_FRAMEWORK_AUDIO_DECODER_ERROR_STATE =                     -10006002,

    /**
     * 输入的音频packet数据不合法
     */
    CICADA_FRAMEWORK_AUDIO_DECODER_ERROR_INPUT =                     -10006003,

    /**
     * 音频解码器内部缓存已满，需要等待
     */
    CICADA_FRAMEWORK_AUDIO_DECODER_ERROR_NO_BUFFER_AVAILABLE =       -10006004,

    /**
     * 音频解码器内部打开失败
     */
    CICADA_ADEC_ERROR_FFMPEG_OEPN_FAIL =                             -10006005,

    /**
     * 不支持的音频解码类型
     */
    CICADA_ADEC_ERROR_FFMPEG_NOT_FIND_DECODER =                      -10006006,

    /**
     * 音频解码器解码一帧失败
     */
    CICADA_ADEC_ERROR_FFMPEG_DEC_ERROR =                             -10006007,

    /**
     * 音频解码器创建无内存
     */
    CICADA_ADEC_ERROR_FFMPEG_NO_CONTEXT =                            -10006008,

    // render
    /**
     * 渲染错误码起始字段
     */
    CICADA_FRAMEWORK_RENDER_ERROR_START =                            -10007000,

    /**
     * 第一帧已渲染
     */
    CICADA_FRAMEWORK_RENDER_FIRST_FRAME_PREVIEWED =                  -10007001,

    /**
     * EGL报错
     */
    CICADA_FRAMEWORK_RENDER_ERROR_EGL =                              -10007002,

    /**
     * GL报错
     */
    CICADA_FRAMEWORK_RENDER_ERROR_GL =                               -10007003,

    /**
     * 待渲染的数据非法
     */
    CICADA_FRAMEWORK_RENDER_ERROR_INVALID_DATA =                     -10007004,

    /**
     * Deprecated 未使用
     */
    CICADA_FRAMEWORK_RENDER_ERROR_INVALID_OPERATION =                -10007005,

    /**
     * 渲染的窗口大小非法
     */
    CICADA_FRAMEWORK_RENDER_ERROR_EDITORLAYOUT_INVALID_SIZE =        -10007006,

    /**
     * 渲染节点编排异常
     */
    CICADA_FRAMEWORK_RENDER_ERROR_SCENE_INVALID =                    -10007007,

    /**
     * 渲染节点参数设置异常
     */
    CICADA_FRAMEWORK_RENDER_ERROR_INVALID_OPTION =                   -10007008,

    /**
     * 渲染编排构建异常
     */
    CICADA_FRAMEWORK_RENDER_ERROR_LAYOUT_NOT_INIT =                  -10007009,

    /**
     * 帧动画传入参数异常
     */
    CICADA_FRAMEWORK_RENDER_ERROR_INVALID_ANIMATION =                -10007010,

    /**
     * GL上下文创建失败
     */
    CICADA_FRAMEWORK_RENDER_ERROR_GL_CONTEXT_INIT_FAILED =           -10007011,

    /**
     * Deprecated 未使用
     */
    CICADA_FRAMEWORK_RENDER_ERROR_LOAD_LIBRARY_FAILED =              -10007012,

    /**
     * eglSwapBuffers失败
     */
    CICADA_FRAMEWORK_RENDER_ERROR_GL_SWAP_BUFFER_FAILED =            -10007013,

    /**
     * Deprecated 未使用
     */
    CICADA_FRAMEWORK_RENDER_ERROR_GL_SWAP_DEFAULT_BUFFER =           -10007014,

    /**
     * 数据池开始错误码
     */
    CICADA_FRAMEWORK_MEDIA_POOL_ERROR_START =                        -10008000,

    /**
     * 数据池状态错误
     */
    CICADA_FRAMEWORK_MEDIA_POOL_WRONG_STATE =                        -10008001,

    /**
     * 数据池处理失败
     */
    CICADA_FRAMEWORK_MEDIA_POOL_PROCESS_FAILED =                     -10008002,

    /**
     * 磁盘空间不足
     */
    CICADA_FRAMEWORK_MEDIA_POOL_NO_FREE_DISK_SPACE =                 -10008003,

    /**
     * 数据池解析视频GOP出错
     */
    CICADA_FRAMEWORK_MEDIA_POOL_CREATE_DECODE_GOP_TASK_FAILED =      -10008004,

    /**
     * 数据池音频流初始化失败
     */
    CICADA_FRAMEWORK_MEDIA_POOL_AUDIO_STREAM_DECODER_INIT_FAILED =   -10008005,

    /**
     * 数据池视频流初始化失败
     */
    CICADA_FRAMEWORK_MEDIA_POOL_VIDEO_STREAM_DECODER_INIT_FAILED =   -10008006,

    /**
     * 数据池缓冲区溢出
     */
    CICADA_FRAMEWORK_MEDIA_POOL_CACHE_DATA_SIZE_OVERFLOW =           -10008007,

    /**
     * 数据池媒体流不存在
     */
    CICADA_FRAMEWORK_MEDIA_POOL_STREAM_NOT_EXISTS =                  -10008008,

    /**
     * 音频处理起始段
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_ERROR_START =                     -10009000,

    /**
     * 创建失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_CREATE_FAILED =                   -10009001,

    /**
     * 重新创建
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_RE_CREATE =                       -10009002,

    /**
     * 销毁失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_DESTROY_FAILED =                  -10009003,

    /**
     * 创建失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_RE_DESTROY =                      -10009004,

    /**
     * 流配置失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_FILE_STREAM_LIST_FAILED =         -10009005,

    /**
     * 属性配置失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_OPTION_LIST_FAILED =              -10009006,

    /**
     * 送算法模块失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_ADD_FRAME_INPUT_FAILED =          -10009007,

    /**
     * 内存满
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_ADD_FRAME_BUFFER_FULL =           -10009008,

    /**
     * 配置应用失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_CTL_INPUT_ERROR =                 -10009009,

    /**
     * 拖动失败
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_SEEK_FAILED =                     -10009010,

    /**
     * 未知配置
     */
    CICADA_FRAMEWORK_AUDIO_PROCESS_CONTROL_UNKNOWN =                 -10009011,

    /**
     * 音频渲染起始段
     */
    CICADA_FRAMEWORK_AUDIO_RENDER_ERROR_START =                      -10010000,

    /**
     * 内核创建失败
     */
    CICADA_FRAMEWORK_AUDIO_RENDER_INIT_SPEAKER_FAILED =              -10010001,

    /**
     * 重复创建
     */
    CICADA_FRAMEWORK_AUDIO_RENDER_ALLOC_SPEAKER_FAILED =             -10010002,

    /**
     * 设置静音失败
     */
    CICADA_FRAMEWORK_AUDIO_RENDER_SETMUTE_FAILED =                   -10010003,

    /**
     * 短视频开始段
     */
    CICADA_SVIDEO_ERROR_START =                                      -20000000,

    /**
     * 权限相关开始段
     */
    CICADA_SVIDEO_ERROR_PERMISSION_START =                           -20001000,

    /**
     * License无效
     */
    CICADA_SVIDEO_ERROR_LICENSE_FAILED =                             -20001001,

    /**
     * 未支付该特效功能
     */
    CICADA_SVIDEO_ERROR_EFFECT_NOT_PAY =                             -20001002,

    /**
     * 特效使用资源不存在
     */
    CICADA_SVIDEO_ERROR_EFFECT_NO_RESOURCE =                         -20001003,

    /**
     * 特效不能覆盖已有的效果
     */
    CICADA_SVIDEO_ERROR_EFFECT_USE_NOT_OVERRIDE =                    -20001004,

    /**
     * 特效使用失败
     */
    CICADA_SVIDEO_ERROR_EFFECT_USE_FAILED =                          -20001005,

    /**
     * 没有设备权限(音频采集、文件读写、摄像头使用、网络访问等Android 设备权限）
     */
    CICADA_SVIDEO_ERROR_PERM_NO_DEVICE_PERMISSION =                  -20001006,

    /**
     * 设置背景音乐时参数错误，像背景音乐路径及时间参数
     */
    CICADA_SVIDEO_ERROR_MUSIC_PARAM =                                -20001007,

    /**
     * 设置背景音乐时错误，音频格式不支持;
     */
    CICADA_SVIDEO_ERROR_MUSIC_NOT_SUPPORT =                          -20001008,

    /**
     * TRES开始段
     */
    CICADA_SVIDEO_ERROR_TRES_START =                                 -20002000,

    /**
     * 播放器未准备好
     */
    CICADA_SVIDEO_ERROR_TRES_PLAYER_UNPREPARED =                     -20002001,

    /**
     * 多次调用异常
     */
    CICADA_SVIDEO_ERROR_TRES_START_MULTI =                           -20002002,

    /**
     * 音频输入流异常
     */
    CICADA_SVIDEO_ERROR_TRES_AUDIO_INPUTS_INVALID =                  -20002003,

    /**
     * 视频渲染类异常
     */
    CICADA_SVIDEO_ERROR_TRES_SCREENRENDER_INVALID =                  -20002004,

    /**
     * 重复添加特效
     */
    CICADA_SVIDEO_ERROR_TRES_DUPLICATE_ADD_EFFECT =                  -20002005,

    /**
     * 不合法的状态
     */
    CICADA_SVIDEO_ERROR_TRES_INVALID_STATE =                         -20002006,

    /**
     * param开始段
     */
    CICADA_SVIDEO_ERROR_PARAM_START =                                -20003000,

    /**
     * 参数异常
     */
    CICADA_SVIDEO_ERROR_PARAM_PARAMETER =                            -20003001,

    /**
     * 无效参数
     */
    CICADA_SVIDEO_ERROR_PARAM_INVALID_ARGUMENTS =                    -20003002,

    /**
     * 输入路径为空
     */
    CICADA_SVIDEO_ERROR_PARAM_PATH_NULL =                            -20003003,

    /**
     * 输入视频路径为空
     */
    CICADA_SVIDEO_ERROR_PARAM_VIDEO_PATH_NULL =                      -20003004,

    /**
     * 文件不存在
     */
    CICADA_SVIDEO_ERROR_PARAM_FILE_NOT_EXISTS =                      -20003005,

    /**
     * 片段索引无效，超出clip列表最大值或者索引不合法
     */
    CICADA_SVIDEO_ERROR_PARAM_CLIP_INDEX_INVALID =                   -20003006,

    /**
     * 片段无效，片段为null
     */
    CICADA_SVIDEO_ERROR_PARAM_CLIP_INVALID =                         -20003007,

    /**
     * 当前列表已经没有片段了, 不能执行删除操作
     */
    CICADA_SVIDEO_ERROR_PARAM_NO_CLIP =                              -20003008,

    /**
     * 视频尺寸设置不正确
     */
    CICADA_SVIDEO_ERROR_PARAM_SIZE_INVALID =                         -20003009,

    /**
     * 视频或音频时长设置异常
     */
    CICADA_SVIDEO_ERROR_PARAM_AUDIO_VIDEO_DURATION_INVALID =         -20003010,

    /**
     * 动图路径异常
     */
    CICADA_SVIDEO_ERROR_PARAM_GIF_FILE_PATH_INVALID =                -20003011,

    /**
     * 资源解析异常
     */
    CICADA_SVIDEO_ERROR_PARAM_RESOURCE_PARSE_INVALID =               -20003012,

    /**
     * 图片路径异常
     */
    CICADA_SVIDEO_ERROR_PARAM_IMAGE_FILE_PATH_INVALID =              -20003013,

    /**
     * 滤镜路径异常
     */
    CICADA_SVIDEO_ERROR_PARAM_FILTER_FILE_PATH_INVALID =             -20003014,

    /**
     * PictureSize不合法
     */
    CICADA_SVIDEO_ERROR_PARAM_PICTURE_SIZE =                         -20003015,

    /**
     * 水印对象为空
     */
    CICADA_SVIDEO_ERROR_PARAM_IMAGE_WATERMARK_NULL =                 -20003016,

    /**
     * 无效的涂鸦
     */
    CICADA_SVIDEO_ERROR_PARAM_INVALID_CANVAS =                       -20003017,

    /**
     * 特效作用时间区间冲突（两个动效滤镜时间区间有交叉）
     */
    CICADA_SVIDEO_ERROR_PARAM_CONFLICT_TIME_INTERVAL =               -20003018,

    /**
     * project 为空
     */
    CICADA_SVIDEO_ERROR_PARAM_PROJECT_NULL =                         -20003019,

    /**
     * 添加媒体文件失败
     */
    CICADA_SVIDEO_ERROR_PARAM_ADD_MEDIA_SOURCE_FAILED =              -20003020,

    /**
     * 输出路径无效
     */
    CICADA_SVIDEO_ERROR_PARAM_OUTOUT_PATH_INVALID =                  -20003021,

    /**
     * MV路径异常
     */
    CICADA_SVIDEO_ERROR_PARAM_MV_FILE_PATH_INVALID =                 -20003022,

    /**
     * media错误开始段
     */
    CICADA_SVIDEO_ERROR_MEDIA_START =                                -20004000,

    /**
     * 不支持的视频格式
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_VIDEO =                  -20004001,

    /**
     * 不支持的音频格式
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_AUDIO =                  -20004002,

    /**
     * 不支持的图片格式
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_IMAGE =                  -20004003,

    /**
     * 不支持的媒体类型
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_TYPE =                   -20004004,

    /**
     * 视频编码器内部错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_VIDEO_ENCODER_INTERNAL =               -20004005,

    /**
     * 音频编码器内部错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_AUDIO_ENCODER_INTERNAL =               -20004006,

    /**
     * 裁剪图片失败
     */
    CICADA_SVIDEO_ERROR_MEDIA_CROP_IMAGE_FAILURE =                   -20004007,

    /**
     * 视频解码器内部错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_VIDEO_DECODER_INTERNAL =               -20004008,

    /**
     * 视频编码器参数错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_VIDEO_ENCODER_PARAM =                  -20004009,

    /**
     * 音频编码器参数错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_AUDIO_ENCODER_PARAM =                  -20004010,

    /**
     * 音频解码器内部错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_AUDIO_DECODER_INTERNAL =               -20004011,

    /**
     * 不支持的像素格式
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_PIXEL_FORMAT =           -20004012,

    /**
     * 播放器内部错误
     */
    CICADA_SVIDEO_ERROR_MEDIA_PLAYER_INTERNAL =                      -20004013,

    /**
     * 不支持的像素格式
     */
    CICADA_SVIDEO_ERROR_MEDIA_NOT_SUPPORTED_PIXCEL_FORMAT =          -20004014,

    /**
     * 编码组件初始化失败
     */
    CICADA_SVIDEO_ERROR_MEDIA_DECODER_CANNOT_WORK =                  -20004015,

    /**
     * 解析输入文件失败
     */
    CICADA_SVIDEO_ERROR_MEDIA_PARSE_INPUT_FILE_FAILED =              -20004016,

    /**
     * 视频队列为空
     */
    CICADA_SVIDEO_ERROR_MEDIA_IMAGE_QUEQU_EMPTY =                    -20004017,

    /**
     * 音频队列为空
     */
    CICADA_SVIDEO_ERROR_MEDIA_SOUND_QUEQU_EMPTY =                    -20004018,

    /**
     * 音频处理最大仅支持两路输入
     */
    CICADA_SVIDEO_ERROR_MEDIA_AUDIO_PROC_SUPPORT_TWO_INPUTS =        -20004019,

    /**
     * 音频处理初始化失败
     */
    CICADA_SVIDEO_ERROR_MEDIA_AUDIO_PROC_INIT_FAILED =               -20004020,

    /**
     * MV配置文件解析失败
     */
    CICADA_SVIDEO_ERROR_MEDIA_MV_PARSE_CONFIG_ERROR =                -20004021,

    /**
     * IO开始段
     */
    CICADA_SVIDEO_ERROR_IO_START =                                   -20005000,

    /**
     * 开启预览失败
     */
    CICADA_SVIDEO_ERROR_IO_START_PREVIEW_FAILED =                    -20005001,

    /**
     * 开启摄像头失败
     */
    CICADA_SVIDEO_ERROR_IO_OPEN_CAMERA_FAILED =                      -20005002,

    /**
     * 切换摄像头失败
     */
    CICADA_SVIDEO_ERROR_IO_SWITCH_CAMERA =                           -20005003,

    /**
     * 创建临时录制文件失败
     */
    CICADA_SVIDEO_ERROR_IO_CREATE_TEMP_FILE_FAILED =                 -20005004,

    /**
     * 音频录制失败
     */
    CICADA_SVIDEO_ERROR_IO_AUDIO_RECORD_FAILED =                     -20005005,

    /**
     * 摄像头设置参数错误
     */
    CICADA_SVIDEO_ERROR_IO_CAMERA_SET_PARAM =                        -20005006,

    /**
     * 摄像头未开启
     */
    CICADA_SVIDEO_ERROR_IO_CAMERA_NOT_OPEN =                         -20005007,

    /**
     * 不支持的摄像头类型
     */
    CICADA_SVIDEO_ERROR_IO_NOT_SUPPORTED_CAMERA_TYPE =               -20005008,

    /**
     * 其他错误开始段
     */
    CICADA_SVIDEO_ERROR_OTHER_START =                                -20006000,

    /**
     * 录制失败，来电占用
     */
    CICADA_SVIDEO_ERROR_OTHER_PHONE_CALL =                           -20006001,

    /**
     * 退后台报错
     */
    CICADA_SVIDEO_ERROR_OTHER_RESIGN_ACTIVE =                        -20006002,

    /**
     * 缩略图开始段
     */
    CICADA_SVIDEO_ERROR_THUMB_START =                                -20007000,

    /**
     * 缩略图内部逻辑错误
     */
    CICADA_SVIDEO_ERROR_THUMB_INTERNAL_LOGIC =                       -20007001,

    /**
     * 缩略图色彩空间转换初始化失败
     */
    CICADA_SVIDEO_ERROR_THUMB_IMAGE_MATRIX_INIT_FAILED =             -20007002,

    /**
     * 缩略图色彩空间转换失败
     */
    CICADA_SVIDEO_ERROR_THUMB_IMAGE_MATRIX_DETAIL_DATA =             -20007003,

    /**
     * 缩略图解码器内部错误
     */
    CICADA_SVIDEO_ERROR_THUMB_DECODER_INTERNAL =                     -20007004,

    /**
     * 缩略图流解析错误
     */
    CICADA_SVIDEO_ERROR_THUMB_VIDEO_STREAM =                         -20007005,

    /**
     * 保留错误码
     */
    CICADA_SVIDEO_ERROR_THUMB_TASK_FAILED =                          -20007006,

    /**
     * 缩略图准备失败
     */
    CICADA_SVIDEO_ERROR_THUMB_PREPARE_FAILED =                       -20007007,

    /**
     * 录制开始段
     */
    CICADA_SVIDEO_ERROR_RECORD_START =                               -20008000,

    /**
     * 视频输入句柄设置为空
     */
    CICADA_SVIDEO_ERROR_RECORD_VIDEO_RGB_FAILED =                    -20008001,

    /**
     * 视频输入句柄设置为空
     */
    CICADA_SVIDEO_ERROR_RECORD_VIDEO_DATA_FAILED =                   -20008002,

    /**
     * 音频输入句柄设置为空
     */
    CICADA_SVIDEO_ERROR_RECORD_AUDIO_DATA_FAILED =                   -20008003,

    /**
     * 结束录制失败
     */
    CICADA_SVIDEO_ERROR_RECORD_FINISH_FAILED =                       -20008004,

    /**
     * 停止录制未完成
     */
    CICADA_SVIDEO_ERROR_RECORD_NOT_COMPLETE =                        -20008005,

    /**
     * 录制编码器初始化失败
     */
    CICADA_SVIDEO_ERROR_RECORD_ENCODER_INIT_FAILED =                 -20008006,

    /**
     * 录制未准备好
     */
    CICADA_SVIDEO_ERROR_RECORD_RECORDER_NOT_READY =                  -20008007,

    /**
     * 录制已达到最大时长
     */
    CICADA_SVIDEO_ERROR_RECORD_MAX_DURATION =                        -20008008,

    /**
     * 合拍错误码起始字段
     */
    CICADA_SVIDEO_ERROR_MIX_START =                                  -20009000,

    /**
     * 合拍渲染布局失败
     */
    CICADA_SVIDEO_ERROR_MIX_LAYOUT =                                 -20009001,

    /**
     * 转码错误码起始字段
     */
    CICADA_SVIDEO_ERROR_TRANSCODE_START =                            -20010000,

    /**
     * 转码状态错误
     */
    CICADA_SVIDEO_ERROR_TRANSCODE_ILLEGAL_CROP_STATE =               -20010001,

    /**
     * 错误的转码参数
     */
    CICADA_SVIDEO_ERROR_TRANSCODE_CROP_PARAM =                       -20010002,

    /**
     * 转码退后台错误
     */
    CICADA_SVIDEO_ERROR_TRANSCODE_BACKGROUND =                       -20010003,

    /**
     * 转码初始化参数无效
     */
    CICADA_SVIDEO_ERROR_TRANSCODE_INIT_PARAM_INVALID =               -20010004,

    // editor
    /**
     * 编辑错误码起始字段
     */
    CICADA_SVIDEO_ERROR_EDITOR_ERROR_START =                         -20011000,

    /**
     * 编辑模块系统性错误，如编解码错误
     */
    CICADA_SVIDEO_ERROR_EDITOR_SYSTEM =                              -20011001,

    /**
     * 编辑准备阶段出错
     */
    CICADA_SVIDEO_ERROR_EDITOR_PREPARE =                             -20011002,

    /**
     * 编辑开始阶段出错（开始播放，开始合成）
     */
    CICADA_SVIDEO_ERROR_EDITOR_START =                               -20011003,

    /**
     * 编辑停止阶段出错（停止播放，停止合成）
     */
    CICADA_SVIDEO_ERROR_EDITOR_STOP =                                -20011004,

    /**
     * 编辑暂停阶段出错（暂停播放，暂停合成）
     */
    CICADA_SVIDEO_ERROR_EDITOR_PAUSE =                               -20011005,

    /**
     * 编辑继续阶段出猝（继续播放，继续合成）
     */
    CICADA_SVIDEO_ERROR_EDITOR_RESUME =                              -20011006,

    /**
     * 编辑快进出错
     */
    CICADA_SVIDEO_ERROR_EDITOR_SEEK =                                -20011007,

    /**
     * 编辑时间特效不支持多段视频，包括时间特效反复，倒播
     */
    CICADA_SVIDEO_ERROR_EDITOR_TIME_EFFECT_NOT_SUPPORT =             -20011008,

    /**
     * 编辑初始化失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_INIT_FAILED =                         -20011009,

    /**
     * 编辑切换模式，反初始化前一种模式的组件失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_SET_MODE_UNINIT_FAILED =              -20011010,

    /**
     * 编辑设置模式时没有找到对应的显示输出设置
     */
    CICADA_SVIDEO_ERROR_EDITOR_SET_MODE_DISPLAY_NONE  =              -20011011,

    /**
     * 快进间隔时间太小
     */
    CICADA_SVIDEO_ERROR_EDITOR_SEEK_IN_OFFSET =                      -20011012,

    /**
     * 编辑设置显示参数失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_SET_DISPLAY_FAILED =                  -20011013,

    /**
     * 编辑时找不到对应的文件
     */
    CICADA_SVIDEO_ERROR_EDITOR_FILE_NOT_EXIST =                      -20011014,

    /**
     * 特效资源解析失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_PARSE_RESOURCE_FAILED =               -20011015,

    /**
     * 不支持的图片特效类型
     */
    CICADA_SVIDEO_ERROR_EDITOR_VIEW_TYPE_NOT_SUPPORTED =             -20011016,

    /**
     * Demuxer解析文件失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_DEMUX_FILE_FAILED =                   -20011017,

    /**
     * 合成状态出错
     */
    CICADA_SVIDEO_ERROR_EDITOR_COMPOSE_STATUS_ERROR =                -20011018,

    /**
     * 编辑状态错误
     */
    CICADA_SVIDEO_ERROR_EDITOR_ILLEGAL_EDITOR_STATE =                -20011019,

    /**
     * 无效的合成,合成需要的参数字符串为空
     */
    CICADA_SVIDEO_ERROR_EDITOR_INVALID_COMPOSE =                     -20011020,

    /**
     * 合成初始化失败
     */
    CICADA_SVIDEO_ERROR_EDITOR_COMPOSE_INIT_FAILED =                 -20011021,

    /**
     * 无效的转场时长
     */
    CICADA_SVIDEO_ERROR_EDITOR_TRANSITION_DURATION_INVALID =         -20011022,

    /**
     * 合成需要的组件为空，一般是状态不对
     */
    CICADA_SVIDEO_ERROR_EDITOR_COMPOSE_NULL =                        -20011023,

    /**
     * 编辑未初始化
     */
    CICADA_SVIDEO_ERROR_EDITOR_NATIVE_NOT_INITED =                   -20011024,

    /**
     * 上传起始错误码
     */
    CICADA_SVIDEO_ERROR_UPLOAD_START =                               -20012000,

    /**
     * 上传失败
     */
    CICADA_SVIDEO_ERROR_UPLOAD_FAILED =                              -20012001,

    /**
     * 上传参数为空
     */
    CICADA_SVIDEO_ERROR_UPLOAD_VOD_NULL =                            -20012002,

    /**
     * 上传状态错误
     */
    CICADA_SVIDEO_ERROR_UPLOAD_VOD_STATE =                           -20012003,

    // player
    CICADA_PLAYER_ERROR_START =                                      -30000000,

    // live
    CICADA_LIVE_ERROR_START =                                        -40000000,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_START =                          -40000001,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_INVALID_STATE =                  -40000002,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_INIT_FAILED =                    -40000003,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_CAMERA_OPEN_FAILED =             -40000004,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_MIC_OPEN_FAILED =                -40000005,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_AUDIO_START_FAIL =               -40000006,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_VIDEO_START_FAIL =               -40000007,
    CICADA_PUSHER_ERROR_SDK_CAPTURE_ENUMERATION_FAILED =             -40000008,

};

enum CicadaEventType
{
    /*--------- event（warning or notification, not error）---------*/

    CICADA_FRAMEWORK_EVENT_START =                                   10000000,

    // video encoder event
    CICADA_FRAMEWORK_VIDEO_ENCODER_EVENT_START                     = 10003000,
    CICADA_FRAMEWORK_VIDEO_ENCODER_QUEUE_EMPTY_WARNING             = 10003001,
    CICADA_FRAMEWORK_VIDEO_ENCODER_QUEUE_FULL_WARNING              = 10003002,
    CICADA_VDEC_FLUSHASYNC_FINISHED                                = 10003003,
    CICADA_VDEC_WARNING_TRY_AGAIN                                  = 10003004,
    CICADA_VDEC_WARNING_DECODE_INVALID_DATA                        = 10003005,
    CICADA_VDEC_WARNING_DECODE_INVALID_DATA_CB                     = 10003006,
    // video decoder event

    CICADA_FRAMEWORK_VIDEO_DECODER_EVENT_START =                     10004000,
    CICADA_FRAMEWORK_VIDEO_DECODER_WARNING_DECODE_FAILED =           10004001,
    CICADA_FRAMEWORK_VIDEO_DECODER_QUEUE_FULL_WARNING =              10004002,
    CICADA_FRAMEWORK_VIDEO_DECODER_QUEUE_EMPTY_WARNING =             10004003,
    CICADA_VDEC_WARNING_IOS_BACKGROUND                             = 10004301,
    CICADA_VDEC_WARNING_IOS_RESIGN_ACTIVE                          = 10004302,
    CICADA_VDEC_WARNING_IOS_BECOME_ACTIVE                          = 10004303,

    // audio decoder event
    CICADA_FRAMEWORK_AUDIO_DECODER_EVENT_START =                     10006000,
    CICADA_FRAMEWORK_AUDIO_DECODER_WARNING_DECODE_FAILED =           10006001,
    CICADA_FRAMEWORK_AUDIO_DECODER_QUEUE_EMPTY_WARNING =             10006002,
    CICADA_FRAMEWORK_AUDIO_DECODER_QUEUE_FULL_WARNING =              10006003,
    CICADA_ADEC_WARNING_DECODE_INVALID_DATA =                        10006004,

    // short video event
    CICADA_SVIDEO_EVENT_START =                                      20000000,

    // editor event
    CICADA_SVIDEO_EDITOR_EVENT_START =                               20011000,
    CICADA_SVIDEO_EDITOR_EVENT_PLAY_COMPLETE =                       20011001,
    CICADA_SVIDEO_EDITOR_EVENT_PREPARE_COMPLETE =                    20011002,
    CICADA_SVIDEO_EDITOR_EVENT_CURRENT_PLAY_TIME =                   20011003,
    CICADA_SVIDEO_EDITOR_EVENT_MEDIA_POOL_ERROR =                    20011004,
    CICADA_SVIDEO_EDITOR_EVENT_MEDIA_POOL_DATA_PREPARED =            20011005,

    /* player event */
    CICADA_PLAYER_EVENT_START =                                      30000000,

    /* live event */
    CICADA_LIVE_EVENT_START =                                        40000000,
    CICADA_PUSHER_EVENT_CAPTURE_START =                              40000001,
    CICADA_PUSHER_EVENT_CAPTURE_OPEN_CAMERA_SUCCESS =                40000002,
    CICADA_PUSHER_EVENT_CAPTURE_OPEN_MIC_SUCCESS =                   40000003,
    CICADA_PUSHER_EVENT_CAPTURE_CLOSE_CAMERA_SUCCESS =               40000004,
};

const char* GetErrorMessage(CicadaErrorType type);

const char* GetErrorSimpleMessage(CicadaErrorType type);


}// namespace cicada

#endif // CICADA_ERROR_EVENT_DEF_H
