package com.cicada.player.utils.media;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCrypto;
import android.media.MediaFormat;
import android.os.Build;
import android.text.TextUtils;
import android.view.Surface;

import com.cicada.player.utils.Logger;
import com.cicada.player.utils.NativeUsed;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import static android.media.MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
import static android.media.MediaCodec.BUFFER_FLAG_END_OF_STREAM;
import static android.media.MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED;
import static android.media.MediaCodec.INFO_OUTPUT_FORMAT_CHANGED;
import static android.media.MediaCodec.INFO_TRY_AGAIN_LATER;

@NativeUsed
public class MediaCodecDecoder {
    private static final String TAG = MediaCodecDecoder.class.getSimpleName();

    private static final Object queLock = new Object();

    private static final int ERROR = -1;
    private static final int TRY_AGAIN = -11;

    private static int CODEC_CATEGORY_VIDEO = 0;
    private static int CODEC_CATEGORY_AUDIO = 1;

    private Map<String, byte[]> mCodecSpecificDataMap = new LinkedHashMap<>();

    private String mMime;
    private int mCodecCateGory = CODEC_CATEGORY_VIDEO;

    private MediaCodec mMediaCodec = null;
    private MediaCrypto mediaCrypto = null;

    public MediaCodecDecoder() {
    }

    @NativeUsed
    public void setCodecSpecificData(Object datas) {
        Logger.d(TAG, "--> setCodecSpecificData datas " + datas);
        mCodecSpecificDataMap.clear();
        if (datas == null) {
            return;
        }

        mCodecSpecificDataMap.putAll((LinkedHashMap<? extends String, ? extends byte[]>) datas);
    }

    @NativeUsed
    public boolean setDrmInfo(String uuid, byte[] sessionId) {
        Logger.d(TAG, "--> setDrmInfo uuid " + uuid);
        try {
            UUID drmUUID = UUID.fromString(uuid);
            mediaCrypto = new MediaCrypto(drmUUID, sessionId);
        } catch (Exception e) {
            Logger.e(TAG, "createMediaCrypto failed: " + e.getMessage());
            return false;
        }
        return true;
    }

    private boolean forceInsecureDecoder = false;

    @NativeUsed
    public void setForceInsecureDecoder(boolean force) {
        Logger.d(TAG, "--> setForceInsecureDecoder  " + force);
        forceInsecureDecoder = force;
    }

    @NativeUsed
    public int configureVideo(String mime, int width, int height, int angle, Object surface) {
        Logger.d(TAG, "--> configureVideo start " + mime + ", " + width + ", " + height + ", " + surface);
        mCodecCateGory = CODEC_CATEGORY_VIDEO;
        mMime = mime;

        MediaFormat videoFormat = MediaFormat.createVideoFormat(mime, width, height);
        addCsdInfo(videoFormat);
        if (angle != 0) {
            videoFormat.setInteger("rotation-degrees", angle);
        }

        String codecName = findDecoderName(videoFormat);
        if (TextUtils.isEmpty(codecName)) {
            Logger.e(TAG, "not found video codec : " + mime);
            return -12;
        }

        try {
            mMediaCodec = MediaCodec.createByCodecName(codecName);
        } catch (IOException e) {
        }

        if (mMediaCodec == null) {
            Logger.e(TAG, "createByCodecName fail : " + codecName);
            return -13;
        }

        try {
            if (surface instanceof Surface) {
                mMediaCodec.configure(videoFormat, (Surface) surface, mediaCrypto, 0);
            } else {
                mMediaCodec.configure(videoFormat, null, mediaCrypto, 0);
            }
        } catch (Exception e) {
            Logger.e(TAG, "configure fail : " + e.getMessage());
            return -14;
        }

        return 0;
    }

    private String findDecoderName(MediaFormat videoFormat) {
        boolean needSecureDecoder = false;
        if (mediaCrypto != null) {
            needSecureDecoder = !forceInsecureDecoder && mediaCrypto.requiresSecureDecoderComponent(mMime);
        }

        String codecName = getDecoderName(videoFormat, needSecureDecoder);

        if (TextUtils.isEmpty(codecName) && needSecureDecoder) {
            needSecureDecoder = false;
            codecName = getDecoderName(videoFormat, needSecureDecoder);
        }

        Logger.d(TAG, "findDecoderName : " + codecName + " , secure = " + needSecureDecoder);
        return codecName;
    }

    private String getDecoderName(MediaFormat videoFormat, boolean needSecureDecoder) {
        List<MediaCodecInfo> mediaCodecInfoList = MediaCodecUtils.getCodecInfos(mMime, needSecureDecoder, videoFormat);
        String codecName = getNotBlackCodecName(mediaCodecInfoList);

        if (TextUtils.isEmpty(codecName) && !mediaCodecInfoList.isEmpty()) {
            codecName = mediaCodecInfoList.get(0).getName();
        }

        return codecName;
    }

    private String getNotBlackCodecName(List<MediaCodecInfo> mediaCodecInfoList) {
        String codecName = null;
        if (!mediaCodecInfoList.isEmpty()) {
            for (MediaCodecInfo info : mediaCodecInfoList) {
                if (!isBlackCodec(info)) {
                    codecName = info.getName();
                    break;
                }
            }
        }
        return codecName;
    }

    @NativeUsed
    public int configureAudio(String mime, int sampleRate, int channelCount, int isADTS) {
        Logger.d(TAG, "--> configureAudio start " + mime + sampleRate + " , " + channelCount);
        mCodecCateGory = CODEC_CATEGORY_AUDIO;
        mMime = mime;

        MediaFormat audioFormat = MediaFormat.createAudioFormat(mime, sampleRate, channelCount);
        audioFormat.setInteger(MediaFormat.KEY_IS_ADTS, isADTS);
        addCsdInfo(audioFormat);

        String codecName = findDecoderName(audioFormat);
        if (TextUtils.isEmpty(codecName)) {
            Logger.e(TAG, "not found codec : " + mime);
            return -12;
        }

        try {
            mMediaCodec = MediaCodec.createByCodecName(codecName);
        } catch (IOException e) {
        }

        if (mMediaCodec == null) {
            Logger.e(TAG, "createByCodecName fail : " + codecName);
            return -13;
        }

        try {
            mMediaCodec.configure(audioFormat, null, mediaCrypto, 0);
        } catch (Exception e) {
            Logger.e(TAG, "configure fail : " + e.getMessage());
            return -14;
        }

        return 0;
    }

    private void addCsdInfo(MediaFormat format) {
        if (mCodecSpecificDataMap.isEmpty()) {
            return;
        }

        for (String key : mCodecSpecificDataMap.keySet()) {
            byte[] data = mCodecSpecificDataMap.get(key);
            if (data == null) {
                continue;
            }

            ByteBuffer csd = ByteBuffer.allocate(data.length);
            csd.put(data);
            csd.flip();
            format.setByteBuffer(key, csd);
        }
    }

    private ByteBuffer[] mInputBuffers = null;
    private ByteBuffer[] mOutputBuffers = null;
    private MediaCodec.BufferInfo mBufferInfo = null;

    private boolean started = false;

    @NativeUsed
    public int start() {
        Logger.d(TAG, "--> start ");

        if (mMediaCodec == null) {
            Logger.e(TAG, "mMediaCodec  null ");
            return ERROR;
        }

        try {
            mMediaCodec.start();
            started = true;
        } catch (Exception e) {
            Logger.e(TAG, mMediaCodec.getName() + " start fail : " + e.getMessage());
            return ERROR;
        }

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            mInputBuffers = mMediaCodec.getInputBuffers();
            mOutputBuffers = mMediaCodec.getOutputBuffers();
        }

        mBufferInfo = new MediaCodec.BufferInfo();

        return 0;
    }

    @NativeUsed
    public int flush() {
        Logger.d(TAG, "--> flush start");

        if (mMediaCodec == null) {
            Logger.e(TAG, "mMediaCodec  null ");
            return ERROR;
        }
        try {
            mMediaCodec.flush();
        } catch (Exception e) {
            Logger.e(TAG, "flush  fail " + e.getMessage());
        }

        return 0;
    }

    public int stop() {
        Logger.d(TAG, "--> stop start");

        if (mMediaCodec == null) {
            return ERROR;
        }

        if (started) {
            try {
                mMediaCodec.stop();
            } catch (Exception e) {
                Logger.e(TAG, "stop fail " + e.getMessage());
                return ERROR;
            }
        }
        return 0;
    }

    @NativeUsed
    public int release() {
        Logger.d(TAG, "--> release ");

        if (mMediaCodec == null) {
            return ERROR;
        }

        mMediaCodec.release();
        mMediaCodec = null;

        if (mediaCrypto != null) {
            mediaCrypto.release();
        }

        return 0;
    }

    @NativeUsed
    public int releaseOutputBuffer(int index, boolean render) {

        if (mMediaCodec == null) {
            return ERROR;
        }

        try {
            mMediaCodec.releaseOutputBuffer(index, render);
        } catch (Exception e) {
            Logger.e(TAG, "releaseOutputBuffer fail " + e.getMessage());
            return ERROR;
        }
        return 0;
    }

    @NativeUsed
    public int dequeueInputBufferIndex(long timeoutUs) {

        if (mMediaCodec == null) {
            return ERROR;
        }

        try {
            int index = mMediaCodec.dequeueInputBuffer(timeoutUs);
            if (index >= 0) {
                return index;
            } else {
                return TRY_AGAIN;
            }
        } catch (Exception e) {
            Logger.e(TAG, "dequeueInputBufferIndex fail " + e.getMessage());
            return ERROR;
        }

    }

    @NativeUsed
    public int queueInputBuffer(int index, byte[] buffer, long pts, boolean isConfig) {
        return queueInputBufferInner(index, buffer, pts, isConfig, false, null);
    }

    @NativeUsed
    public int queueSecureInputBuffer(int index, byte[] buffer, Object encryptionInfo, long pts, boolean isConfig) {
        return queueInputBufferInner(index, buffer, pts, isConfig, true, encryptionInfo);
    }

    private int queueInputBufferInner(int index, byte[] buffer, long pts, boolean isConfig, boolean secure, Object encryptionInfo) {

        if (mMediaCodec == null) {
            return ERROR;
        }

        ByteBuffer inputBuffer = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            inputBuffer = mMediaCodec.getInputBuffer(index);
        } else {
            inputBuffer = mInputBuffers[index];
        }
        if (inputBuffer == null) {
            return ERROR;
        }

        inputBuffer.clear();

        if (buffer != null) {
            inputBuffer.put(buffer, 0, buffer.length);
            inputBuffer.flip();
        }

        int flags = 0;
        if (isConfig) {
            flags |= BUFFER_FLAG_CODEC_CONFIG;
        }
        if (buffer == null) {
            flags |= BUFFER_FLAG_END_OF_STREAM;
        }

        try {
            if (secure && buffer != null) {
                MediaCodec.CryptoInfo crypInfo = createCryptoInfo((EncryptionInfo) encryptionInfo);
                synchronized (queLock) {
                    mMediaCodec.queueSecureInputBuffer(index, 0, crypInfo, pts, flags);
                }
            } else {
                if ((flags & BUFFER_FLAG_END_OF_STREAM) == BUFFER_FLAG_END_OF_STREAM) {
                    mMediaCodec.queueInputBuffer(index, 0, 0, 0, flags);
                } else {
                    mMediaCodec.queueInputBuffer(index, 0, inputBuffer.limit(), pts, flags);
                }
            }
        } catch (Exception e) {
            Logger.e(TAG, "queueInputBufferInner  fail " + e.getLocalizedMessage());
            return ERROR;
        }

        return 0;

    }

    private MediaCodec.CryptoInfo createCryptoInfo(EncryptionInfo info) {
        MediaCodec.CryptoInfo crypInfo = new MediaCodec.CryptoInfo();
        crypInfo.key = info.key_id;
        crypInfo.iv = info.iv;
        if (info.subsamples != null) {
            crypInfo.numSubSamples = info.subsamples.size();
            crypInfo.numBytesOfClearData = new int[crypInfo.numSubSamples];
            crypInfo.numBytesOfEncryptedData = new int[crypInfo.numSubSamples];
            for (int i = 0; i < info.subsamples.size(); i++) {
                SubsampleEncryptionInfo subsampleEncryptionInfo = info.subsamples.get(i);
                crypInfo.numBytesOfClearData[i] = subsampleEncryptionInfo.bytes_of_clear_data;
                crypInfo.numBytesOfEncryptedData[i] = subsampleEncryptionInfo.bytes_of_protected_data;
            }
        }

        if ("cenc".equals(info.scheme) || "cens".equals(info.scheme)) {
            crypInfo.mode = 1;//AES_CTR
        } else if ("cbc1".equals(info.scheme) || "cbcs".equals(info.scheme)) {
            crypInfo.mode = 2;//AES_CBC
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            if ("cens".equals(info.scheme) || "cbcs".equals(info.scheme)) {
                //has pattern
                int blocksToEncrypt = info.crypt_byte_block;
                int blocksToSkip = info.skip_byte_block;
                MediaCodec.CryptoInfo.Pattern pattern = null;
                pattern = new MediaCodec.CryptoInfo.Pattern(blocksToEncrypt, blocksToSkip);
                crypInfo.setPattern(pattern);
            }
        }
        return crypInfo;
    }

    @NativeUsed
    public int dequeueOutputBufferIndex(long timeoutUs) {
        if (mMediaCodec == null) {
            return ERROR;
        }

        try {
            int index = mMediaCodec.dequeueOutputBuffer(mBufferInfo, timeoutUs);
            if (index >= 0) {
                return index;
            } else if (index == INFO_TRY_AGAIN_LATER) {
                return TRY_AGAIN;
            }else if(index == INFO_OUTPUT_FORMAT_CHANGED) {
                return index;
            } else if (index == INFO_OUTPUT_BUFFERS_CHANGED) {
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                    mOutputBuffers = mMediaCodec.getOutputBuffers();
                }
                return index;
            } else {
                return ERROR;
            }
        } catch (Exception e) {
            Logger.e(TAG, "dequeueOutputBufferIndex fail " + e.getMessage());
            return ERROR;
        }
    }

    @NativeUsed
    public Object getOutputBufferInfo(int index) {
        if (index == INFO_OUTPUT_FORMAT_CHANGED) {
            OutputBufferInfo outputBufferInfo = fillFormatOutputBufferInfo();
            return outputBufferInfo;
        } else if (index >= 0) {
            OutputBufferInfo outputBufferInfo = fillDecodeBufferInfo(index);
            return outputBufferInfo;
        }
        return null;
    }

    @NativeUsed
    public Object getOutBuffer(int index) {
        if (index >= 0) {
            ByteBuffer outputBuffer = null;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                outputBuffer = mMediaCodec.getOutputBuffer(index);
            } else {
                outputBuffer = mOutputBuffers[index];
            }
            return outputBuffer;
        }
        return null;
    }

    private OutputBufferInfo fillDecodeBufferInfo(int index) {
        OutputBufferInfo info = new OutputBufferInfo();
        info.type = 0;//buffer
        info.index = index;
        info.pts = mBufferInfo.presentationTimeUs;
        info.eos = ((mBufferInfo.flags & BUFFER_FLAG_END_OF_STREAM) != 0);
        info.bufferSize = mBufferInfo.size;
        info.bufferOffset = mBufferInfo.offset;
        return info;
    }

    private OutputBufferInfo fillFormatOutputBufferInfo() {

        MediaFormat format = null;
        try {
            format = mMediaCodec.getOutputFormat();
        } catch (Exception e) {
            return null;
        }

        OutputBufferInfo info = new OutputBufferInfo();
        info.type = 1;//config
        info.eos = false;
        if (mCodecCateGory == CODEC_CATEGORY_VIDEO) {
            info.videoCropBottom = getFormatInteger(format, "crop-bottom");
            info.videoCropLeft = getFormatInteger(format, "crop-left");
            info.videoCropRight = getFormatInteger(format, "crop-right");
            info.videoCropTop = getFormatInteger(format, "crop-top");
            info.videoHeight = getFormatInteger(format, MediaFormat.KEY_WIDTH);
            info.videoWidth = getFormatInteger(format, MediaFormat.KEY_HEIGHT);
            info.videoPixelFormat = getFormatInteger(format, MediaFormat.KEY_COLOR_FORMAT);
            info.videoSliceHeight = getFormatInteger(format, "slice-height");
            info.videoStride = getFormatInteger(format, "stride");
        } else {
            info.audioChannelCount = getFormatInteger(format, MediaFormat.KEY_CHANNEL_COUNT);
            info.audioChannelMask = getFormatInteger(format, MediaFormat.KEY_CHANNEL_MASK);
            info.audioSampleRate = getFormatInteger(format, MediaFormat.KEY_SAMPLE_RATE);
            info.audioFormat = getFormatInteger(format, "pcm-encoding");
        }

        return info;
    }

    private static int getFormatInteger(MediaFormat format, String key) {
        if (format.containsKey(key)) {
            return format.getInteger(key);
        }
        return -1;
    }

    static List<String> blackCodecPrefix = new ArrayList<>();
    static List<String> blackCodecSuffix = new ArrayList<>();

    private static boolean isBlackCodec(MediaCodecInfo info) {
        if (blackCodecPrefix.isEmpty()) {
            blackCodecPrefix.add("OMX.PV.");
            blackCodecPrefix.add("OMX.google.");
            blackCodecPrefix.add("OMX.ARICENT.");
            blackCodecPrefix.add("OMX.SEC.WMV.Decoder");
            blackCodecPrefix.add("OMX.SEC.MP3.Decoder");
            blackCodecPrefix.add("OMX.MTK.VIDEO.DECODER.VC1");
            blackCodecPrefix.add("OMX.SEC.vp8.dec");
        }

        if (blackCodecSuffix.isEmpty()) {
            blackCodecSuffix.add(".sw.dec");
            blackCodecSuffix.add(".hevcswvdec");
        }

        String name = info.getName();
        for (String prefix : blackCodecPrefix) {
            if (name.startsWith(prefix)) {
                return true;
            }
        }

        for (String suffix : blackCodecSuffix) {
            if (name.endsWith(suffix)) {
                return true;
            }
        }

        return false;
    }
}
