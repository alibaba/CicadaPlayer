#ifndef CicadaErrorCode_h
#define CicadaErrorCode_h

typedef enum CicadaErrorCode: NSUInteger {
    // 3、demuxer、datasource模块错误
    // 3. demuxer and datasource module errors.
    CICADA_ERROR_DEMUXER_START                                       = 0x20030000,
    CICADA_ERROR_DEMUXER_OPENURL                                     = 0x20030001,
    CICADA_ERROR_DEMUXER_NO_VALID_STREAM                             = 0x20030002,
    CICADA_ERROR_DEMUXER_OPENSTREAM                                  = 0x20030003,
    CICADA_ERROR_LOADING_TIMEOUT                                     = 0x20030004,
    CICADA_ERROR_DATASOURCE_EMPTYURL                                 = 0x20030005,

    CICADA_ERROR_DECODE_BASE                                         = 0x20040000,
    CICADA_ERROR_DECODE_VIDEO                                        = 0x20040001,
    CICADA_ERROR_DECODE_AUDIO                                        = 0x20040002,

    // 4. network错误
    // 4. Network errors.
    CICADA_ERROR_NETWORK_UNKNOWN                                     = 0x20050000,
    CICADA_ERROR_NETWORK_UNSUPPORTED                                 = 0x20050001,
    CICADA_ERROR_NETWORK_RESOLVE                                     = 0x20050002,
    CICADA_ERROR_NETWORK_CONNECT_TIMEOUT                             = 0x20050003,
    CICADA_ERROR_NETWORK_COULD_NOT_CONNECT                           = 0x20050004,
    CICADA_ERROR_NETWORK_HTTP_403                                    = 0x20050005,
    CICADA_ERROR_NETWORK_HTTP_404                                    = 0x20050006,
    CICADA_ERROR_NETWORK_HTTP_4XX                                    = 0x20050007,
    CICADA_ERROR_NETWORK_HTTP_5XX                                    = 0x20050008,
    CICADA_ERROR_NETWORK_HTTP_RANGE                                  = 0x20050009,
    CICADA_ERROR_NETWORK_HTTP_400                                    = 0x2005000A,


    //5. codec 错误
    //5. Codec errors.
    CICADA_ERROR_CODEC_UNKNOWN                                       = 0x20060000,
    CICADA_ERROR_CODEC_VIDEO_NOT_SUPPORT                             = 0x20060001,
    CICADA_ERROR_CODEC_AUDIO_NOT_SUPPORT                             = 0x20060002,

    //6. internal 错误
    //6. Internal errors.
    CICADA_ERROR_INERNAL_UNKNOWN                                     = 0x20070000,
    CICADA_ERROR_INERNAL_EXIT                                        = 0x20070001,

    //7.general 错误
    //7. General errors.
    CICADA_ERROR_GENERAL_UNKNOWN                                     = 0x20080000,
    CICADA_ERROR_GENERAL_EPERM                                       = 0x20080001,
    CICADA_ERROR_GENERAL_ENOENT                                      = 0x20080002,
    CICADA_ERROR_GENERAL_EIO                                         = 0x20080005,

    //
    CICADA_ERROR_PLAYER_UNKNOWN                                      = 0x30000000 - 1,

} CicadaErrorCode;

/**@brief 播放器事件类型，带描述字符串*/
/****@brief Player event type with a description string.*/
typedef enum CicadaEventWithString: NSUInteger {
    /**@brief 设置了硬解，但是切换为软解。额外信息为描述信息。*/
    /****@brief Hardware decoding is switched to software decoding. Additional information: description.*/
    CICADA_EVENT_SWITCH_TO_SOFTWARE_DECODER = 100,
    /**@brief 音频解码格式不支持。额外信息为描述信息。*/
    /****@brief Audio decoding does not support the specified format. Additional information: description.*/
    CICADA_EVENT_AUDIO_CODEC_NOT_SUPPORT    = 101,
    /**@brief 音频解码器设备失败。额外信息为描述信息。*/
    /****@brief The audio decoder failed. Additional information: description.*/
    CICADA_EVENT_AUDIO_DECODER_DEVICE_ERROR = 102,
    /**@brief 视频解码格式不支持。额外信息为描述信息。*/
    /****@brief Video decoding does not support the specified format. Additional information: description.*/
    CICADA_EVENT_VIDEO_CODEC_NOT_SUPPORT    = 103,
    /**@brief 视频解码器设备失败。额外信息为描述信息。*/
    /****@brief The video decoder failed. Additional information: description.*/
    CICADA_EVENT_VIDEO_DECODER_DEVICE_ERROR = 104,
    /**@brief 视频渲染设备初始化失败。额外信息为描述信息。*/
    /****@brief The video renderer failed. Additional information: description.*/
    CICADA_EVENT_VIDEO_RENDER_INIT_ERROR    = 105,

    /**@brief 网络失败，需要重试。无额外信息。*/
    /****@brief Network connection failed. Try again. No additional information is provided.*/
    CICADA_EVENT_PLAYER_NETWORK_RETRY       = 108,
    /**@brief 缓存成功。无额外信息。*/
    /****@brief Content cached. No additional information is provided.*/
    CICADA_EVENT_PLAYER_CACHE_SUCCESS       = 109,
    /**@brief 缓存失败。额外信息为描述信息。*/
    /****@brief Failed to cache the content. Additional information: description.*/
    CICADA_EVENT_PLAYER_CACHE_ERROR         = 110,
    /**@brief 系统无可用内存来存放媒体数据。*/
    /****@brief The system does not have memory to store the media data.*/
    CICADA_EVENT_PLAYER_LOW_MEMORY          = 111,
    /**@brief 网络重试成功。无额外信息。*/
    /****@brief Network retry successful. No additional information is provided.*/
    CICADA_EVENT_PLAYER_NETWORK_RETRY_SUCCESS             = 113,
    /**@brief 选择字幕错误。*/
    /****@brief select subtitle error.*/
    CICADA_EVENT_MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR = 114,
    /**@brief 后台恢复时所需解码帧过多 */
    /****@brief  decoder recover size is too large for background recovery.*/
    CICADA_EVENT_PLAYER_EVENT_DECODER_RECOVER_SIZE        = 115,
} CicadaEventWithString;

#endif

