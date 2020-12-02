package com.cicada.player.utils.media;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;

public class MediaCodecUtils {

    private static final String TAG = MediaCodecUtils.class.getSimpleName();


    /**
     * @param mime
     * @param secure 是否需要安全解码
     * @return
     */
    public static synchronized List<MediaCodecInfo> getCodecInfos(String mime, boolean secure, MediaFormat format) {

        if (allDecoders == null) {
            allDecoders = getDeviceDecodecs();
        }

        List<MediaCodecInfo> mediaCodecInfos = new ArrayList<>();

        for (MediaCodecInfo mediaCodecInfo : allDecoders) {

            String codecMimeType = getCodecMimeType(mediaCodecInfo, mime);
            if (codecMimeType == null) {
//                Logger.v(TAG, mediaCodecInfo.getName() + " not support mime : " + mime);
                continue;
            }

            MediaCodecInfo.CodecCapabilities capabilities = mediaCodecInfo.getCapabilitiesForType(codecMimeType);
            boolean secureMatch = isSecureSupport(secure, capabilities, codecMimeType);
            if (!secureMatch) {
//                Logger.v(TAG, mediaCodecInfo.getName() + " not support secure : " + secure);
                continue;
            }

            boolean formatSupport = isFormatSupport(format, capabilities, codecMimeType);
            if (!formatSupport) {
//                Logger.v(TAG, mediaCodecInfo.getName() + " not support format : " + format);
                continue;
            }

            mediaCodecInfos.add(mediaCodecInfo);
        }

        return mediaCodecInfos;
    }

    private static boolean isFormatSupport(MediaFormat format, MediaCodecInfo.CodecCapabilities capabilities, String codecMimeType) {
        boolean isVideo = codecMimeType.startsWith("video");
        boolean isAudio = codecMimeType.startsWith("audio");
        if (isVideo) {
            int width = getFormatInteger(format, MediaFormat.KEY_WIDTH, -1);
            int height = getFormatInteger(format, MediaFormat.KEY_HEIGHT, -1);
            if (width <= 0 || height <= 0) {
                return true;
            }

            boolean isFormatSupported = false;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                int tmpWidth = Math.max(width, height);
                int tmpHeight = Math.min(width, height);
                MediaCodecInfo.VideoCapabilities videoCapabilities = capabilities.getVideoCapabilities();
                if (videoCapabilities == null) {
                    isFormatSupported = false;
                } else {
                    isFormatSupported = videoCapabilities.isSizeSupported(tmpWidth, tmpHeight);
                }
            } else {
                isFormatSupported = true;
            }

            return isFormatSupported;

        } else if (isAudio) { // Audio
            boolean isFormatSupported = false;
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                isFormatSupported = true;
            } else {
                MediaCodecInfo.AudioCapabilities audioCapabilities = capabilities.getAudioCapabilities();
                if (audioCapabilities == null) {
                    isFormatSupported = false;
                } else {
                    int sampleRate = getFormatInteger(format, MediaFormat.KEY_SAMPLE_RATE, -1);
                    int channelCount = getFormatInteger(format, MediaFormat.KEY_CHANNEL_COUNT, -1);

                    boolean sampleRateSupported = (sampleRate == -1 || audioCapabilities.isSampleRateSupported(sampleRate));
                    boolean channelCountSupported = (channelCount == -1 || audioCapabilities.getMaxInputChannelCount() >= channelCount);

                    isFormatSupported = sampleRateSupported && channelCountSupported;
                }
            }
            return isFormatSupported;
        }

        return false;

    }

    private static int getFormatInteger(MediaFormat format, String name, int defaultValue) {
        int value = defaultValue;
        if (format.containsKey(name)) {
            value = format.getInteger(name);
        }
        return value;
    }

    private static boolean isSecureSupport(boolean secure, MediaCodecInfo.CodecCapabilities capabilities, String codecMimeType) {

        boolean secureSupported = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            secureSupported = capabilities.isFeatureSupported("secure-playback");
        } else {
            secureSupported = "video/avc".equals(codecMimeType);
        }

        boolean secureRequired = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            secureRequired = capabilities.isFeatureRequired("secure-playback");
        } else {
            secureRequired = false;
        }

        if ((!secure && secureRequired) || (secure && !secureSupported)) {
            return false;
        }

        boolean secureDecodersExplicit = Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP;
        if ((secureDecodersExplicit && secure == secureSupported)
                || (!secureDecodersExplicit && !secure)) {
            return true;
        }
        return true;
    }


    private static List<MediaCodecInfo> allDecoders;

    private static List<MediaCodecInfo> getDeviceDecodecs() {
        List<MediaCodecInfo> allCodecs = new ArrayList<>();

        MediaCodecInfo[] codecInfos = null;
        int codecNums = 0;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.ALL_CODECS);
            codecInfos = mediaCodecList.getCodecInfos();
            codecNums = codecInfos.length;
        } else {
            codecNums = MediaCodecList.getCodecCount();
        }

        for (int i = 0; i < codecNums; i++) {
            MediaCodecInfo info = null;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                info = codecInfos[i];
            } else {
                info = MediaCodecList.getCodecInfoAt(i);
            }

            if (info.isEncoder()) {
                continue;
            } else {
                allCodecs.add(info);
            }
        }
        return allCodecs;
    }

    private static String getCodecMimeType(
            MediaCodecInfo info,
            String mimeType) {
        String[] supportedTypes = info.getSupportedTypes();
        for (String supportedType : supportedTypes) {
            if (supportedType.equalsIgnoreCase(mimeType)) {
                return supportedType;
            }
        }

        return null;
    }

}
