package com.fplayer.flutter_cicadaplayer;

import android.content.Context;
import android.os.Build;
import android.text.TextUtils;
import com.cicada.player.CicadaPlayer;
import com.cicada.player.CicadaPlayerFactory;
import com.cicada.player.bean.ErrorInfo;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.nativeclass.CacheConfig;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.PlayerConfig;
import com.cicada.player.nativeclass.TrackInfo;
import com.cicada.player.utils.Logger;
import com.google.gson.Gson;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.plugin.common.EventChannel;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FlutterCicadaPlayer implements EventChannel.StreamHandler, MethodCallHandler {

    private FlutterPlugin.FlutterPluginBinding mFlutterPluginBinding;

    private final Gson mGson;
    private Context mContext;
    private EventChannel.EventSink mEventSink;
    private EventChannel mEventChannel;
    private CicadaPlayer mCicadaPlayer;
    private MethodChannel mCicadaPlayerMethodChannel;
    private String mSnapShotPath;

    public FlutterCicadaPlayer(FlutterPlugin.FlutterPluginBinding flutterPluginBinding)
    {
        this.mFlutterPluginBinding = flutterPluginBinding;
        this.mContext = flutterPluginBinding.getApplicationContext();
        mGson = new Gson();
        mCicadaPlayer = CicadaPlayerFactory.createCicadaPlayer(mFlutterPluginBinding.getApplicationContext());
        mCicadaPlayerMethodChannel = new MethodChannel(mFlutterPluginBinding.getFlutterEngine().getDartExecutor(), "flutter_cicadaplayer");
        mCicadaPlayerMethodChannel.setMethodCallHandler(this);
        mEventChannel = new EventChannel(mFlutterPluginBinding.getFlutterEngine().getDartExecutor(), "flutter_cicadaplayer_event");
        mEventChannel.setStreamHandler(this);
        initListener(mCicadaPlayer);
    }

    public CicadaPlayer getCicadaPlayer()
    {
        return mCicadaPlayer;
    }

    private void initListener(final CicadaPlayer player)
    {
        player.setOnPreparedListener(new CicadaPlayer.OnPreparedListener() {
            @Override public void onPrepared()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onPrepared");
                mEventSink.success(map);
            }
        });

        player.setOnRenderingStartListener(new CicadaPlayer.OnRenderingStartListener() {
            @Override public void onRenderingStart()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onRenderingStart");
                mEventSink.success(map);
            }
        });

        player.setOnVideoSizeChangedListener(new CicadaPlayer.OnVideoSizeChangedListener() {
            @Override public void onVideoSizeChanged(int width, int height)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onVideoSizeChanged");
                map.put("width", width);
                map.put("height", height);
                mEventSink.success(map);
            }
        });

        //        player.setOnSnapShotListener(new CicadaPlayer.OnSnapShotListener() {
        //            @Override
        //            public void onSnapShot(final Bitmap bitmap, int width, int height) {
        //                final Map<String,Object> map = new HashMap<>();
        //                map.put("method","onSnapShot");
        //                map.put("snapShotPath",mSnapShotPath);
        //
        //                ThreadManager.threadPool.execute(new Runnable() {
        //                    @Override
        //                    public void run() {
        //                        File f = new File(mSnapShotPath);
        //                        FileOutputStream out = null;
        //                        if (f.exists()) {
        //                            f.delete();
        //                        }
        //                        try {
        //                            out = new FileOutputStream(f);
        //                            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
        //                            out.flush();
        //                            out.close();
        //                        } catch (FileNotFoundException e) {
        //                            e.printStackTrace();
        //                        } catch (IOException e) {
        //                            e.printStackTrace();
        //                        }finally{
        //                            if(out != null){
        //                                try {
        //                                    out.close();
        //                                } catch (IOException e) {
        //                                    e.printStackTrace();
        //                                }
        //                            }
        //                        }
        //                    }
        //                });
        //
        //                mEventSink.success(map);
        //
        //            }
        //        });

        player.setOnTrackChangedListener(new CicadaPlayer.OnTrackChangedListener() {
            @Override public void onChangedSuccess(TrackInfo trackInfo)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onTrackChanged");
                Map<String, Object> infoMap = new HashMap<>();
                //                infoMap.put("vodFormat",trackInfo.getVodFormat());
                infoMap.put("videoHeight", trackInfo.getVideoHeight());
                infoMap.put("videoWidth", trackInfo.getVideoHeight());
                infoMap.put("subtitleLanguage", trackInfo.getSubtitleLang());
                infoMap.put("trackBitrate", trackInfo.getVideoBitrate());
                //                infoMap.put("vodFileSize",trackInfo.getVodFileSize());
                infoMap.put("trackIndex", trackInfo.getIndex());
                //                infoMap.put("trackDefinition",trackInfo.getVodDefinition());
                infoMap.put("audioSampleFormat", trackInfo.getAudioSampleFormat());
                infoMap.put("audioLanguage", trackInfo.getAudioLang());
                //                infoMap.put("vodPlayUrl",trackInfo.getVodPlayUrl());
                infoMap.put("trackType", trackInfo.getType().ordinal());
                infoMap.put("audioSamplerate", trackInfo.getAudioSampleRate());
                infoMap.put("audioChannels", trackInfo.getAudioChannels());
                map.put("info", infoMap);
                mEventSink.success(map);
            }

            @Override public void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onChangedFail");
                mEventSink.success(map);
            }
        });

        player.setOnSeekCompleteListener(new CicadaPlayer.OnSeekCompleteListener() {
            @Override public void onSeekComplete()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onSeekComplete");
                mEventSink.success(map);
            }
        });

        //        player.setOnSeiDataListener(new CicadaPlayer.OnSeiDataListener() {
        //            @Override
        //            public void onSeiData(int type, byte[] bytes) {
        //                Map<String,Object> map = new HashMap<>();
        //                map.put("method","onSeiData");
        //                //TODO
        //                mEventSink.success(map);
        //            }
        //        });

        player.setOnLoadingStatusListener(new CicadaPlayer.OnLoadingStatusListener() {
            @Override public void onLoadingBegin()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onLoadingBegin");
                mEventSink.success(map);
            }

            @Override public void onLoadingProgress(int percent, float netSpeed)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onLoadingProgress");
                map.put("percent", percent);
                map.put("netSpeed", netSpeed);
                mEventSink.success(map);
            }

            @Override public void onLoadingEnd()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onLoadingEnd");
                mEventSink.success(map);
            }
        });

        player.setOnStateChangedListener(new CicadaPlayer.OnStateChangedListener() {
            @Override public void onStateChanged(int newState)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onStateChanged");
                map.put("newState", newState);
                mEventSink.success(map);
            }
        });

        player.setOnSubtitleDisplayListener(new CicadaPlayer.OnSubtitleDisplayListener() {
            @Override public void onSubtitleExtAdded(int trackIndex, String url)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onSubtitleExtAdded");
                map.put("trackIndex", trackIndex);
                map.put("url", url);
                mEventSink.success(map);
            }

            @Override public void onSubtitleShow(int trackIndex, long id, String data)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onSubtitleShow");
                map.put("trackIndex", trackIndex);
                map.put("subtitleID", id);
                map.put("subtitle", data);
                mEventSink.success(map);
            }

            @Override public void onSubtitleHide(int trackIndex, long id)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onSubtitleHide");
                map.put("trackIndex", trackIndex);
                map.put("subtitleID", id);
                mEventSink.success(map);
            }
        });

        player.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
            @Override public void onInfo(InfoBean infoBean)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onInfo");
                map.put("infoCode", infoBean.getCode().getValue());
                map.put("extraValue", infoBean.getExtraValue());
                map.put("extraMsg", infoBean.getExtraMsg());
                mEventSink.success(map);
            }
        });

        player.setOnErrorListener(new CicadaPlayer.OnErrorListener() {
            @Override public void onError(ErrorInfo errorInfo)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onError");
                map.put("errorCode", errorInfo.getCode().getValue());
                map.put("errorExtra", errorInfo.getExtra());
                map.put("errorMsg", errorInfo.getMsg());
                mEventSink.success(map);
            }
        });

        player.setOnTrackReadyListener(new CicadaPlayer.OnTrackReadyListener() {
            @Override public void onTrackReady(MediaInfo mediaInfo)
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onTrackReady");
                mEventSink.success(map);
            }
        });

        player.setOnCompletionListener(new CicadaPlayer.OnCompletionListener() {
            @Override public void onCompletion()
            {
                Map<String, Object> map = new HashMap<>();
                map.put("method", "onCompletion");
                mEventSink.success(map);
            }
        });
    }

    @Override public void onListen(Object arguments, EventChannel.EventSink events)
    {
        this.mEventSink = events;
    }

    @Override public void onCancel(Object arguments)
    {}

    @Override public void onMethodCall(MethodCall methodCall, MethodChannel.Result result)
    {
        switch (methodCall.method) {
            case "createCicadaPlayer":
                createCicadaPlayer();
                break;
            case "setUrl":
                String url = methodCall.arguments.toString();
                setDataSource(url);
                break;
            case "prepare":
                prepare();
                break;
            case "play":
                start();
                break;
            case "pause":
                pause();
                break;
            case "stop":
                stop();
                break;
            case "destroy":
                release();
                break;
            case "seekTo": {
                Map<String, Object> seekToMap = (Map<String, Object>) methodCall.arguments;
                Integer position = (Integer) seekToMap.get("position");
                Integer seekMode = (Integer) seekToMap.get("seekMode");
                seekTo(position, seekMode);
            } break;
            case "getMediaInfo": {
                MediaInfo mediaInfo = getMediaInfo();
                if (mediaInfo != null) {
                    Map<String, Object> getMediaInfoMap = new HashMap<>();
                    //                    getMediaInfoMap.put("title",mediaInfo.getTitle());
                    //                    getMediaInfoMap.put("status",mediaInfo.getStatus());
                    //                    getMediaInfoMap.put("mediaType",mediaInfo.getMediaType());
                    //                    getMediaInfoMap.put("duration",mediaInfo.getDuration());
                    //                    getMediaInfoMap.put("transcodeMode",mediaInfo.getTransCodeMode());
                    //                    getMediaInfoMap.put("coverURL",mediaInfo.getCoverUrl());

                    List<TrackInfo> trackInfos = mediaInfo.getTrackInfos();
                    List<Map<String, Object>> trackInfoList = new ArrayList<>();
                    for (TrackInfo trackInfo : trackInfos) {
                        Map<String, Object> map = new HashMap<>();
                        //                        map.put("vodFormat",trackInfo.getVodFormat());
                        map.put("videoHeight", trackInfo.getVideoHeight());
                        map.put("videoWidth", trackInfo.getVideoHeight());
                        map.put("subtitleLanguage", trackInfo.getSubtitleLang());
                        map.put("trackBitrate", trackInfo.getVideoBitrate());
                        //                        map.put("vodFileSize",trackInfo.getVodFileSize());
                        map.put("trackIndex", trackInfo.getIndex());
                        //                        map.put("trackDefinition",trackInfo.getVodDefinition());
                        map.put("audioSampleFormat", trackInfo.getAudioSampleFormat());
                        map.put("audioLanguage", trackInfo.getAudioLang());
                        //                        map.put("vodPlayUrl",trackInfo.getVodPlayUrl());
                        map.put("trackType", trackInfo.getType().ordinal());
                        map.put("audioSamplerate", trackInfo.getAudioSampleRate());
                        map.put("audioChannels", trackInfo.getAudioChannels());
                        trackInfoList.add(map);
                        getMediaInfoMap.put("tracks", trackInfoList);
                    }
                    result.success(getMediaInfoMap);
                }
            } break;
            case "getDuration":
                result.success(mCicadaPlayer.getDuration());
                break;
            case "snapshot":
                mSnapShotPath = methodCall.arguments.toString();
                snapshot();
                break;
            case "setLoop":
                setLoop((Boolean) methodCall.arguments);
                break;
            case "isLoop":
                result.success(isLoop());
                break;
            case "setAutoPlay":
                setAutoPlay((Boolean) methodCall.arguments);
                break;
            case "isAutoPlay":
                result.success(isAutoPlay());
                break;
            case "setMuted":
                setMuted((Boolean) methodCall.arguments);
                break;
            case "isMuted":
                result.success(isMuted());
                break;
            case "setEnableHardwareDecoder":
                Boolean setEnableHardwareDecoderArgumnt = (Boolean) methodCall.arguments;
                setEnableHardWareDecoder(setEnableHardwareDecoderArgumnt);
                break;
            case "setScalingMode":
                setScaleMode((Integer) methodCall.arguments);
                break;
            case "getScalingMode":
                result.success(getScaleMode());
                break;
            case "setMirrorMode":
                setMirrorMode((Integer) methodCall.arguments);
                break;
            case "getMirrorMode":
                result.success(getMirrorMode());
                break;
            case "setRotateMode":
                setRotateMode((Integer) methodCall.arguments);
                break;
            case "getRotateMode":
                result.success(getRotateMode());
                break;
            case "setRate":
                setSpeed((Double) methodCall.arguments);
                break;
            case "getRate":
                result.success(getSpeed());
                break;
            case "setVideoBackgroundColor":
                setVideoBackgroundColor((Long) methodCall.arguments);
                break;
            case "setVolume":
                setVolume((Double) methodCall.arguments);
                break;
            case "getVolume":
                result.success(getVolume());
                break;
            case "setConfig": {
                Map<String, Object> setConfigMap = (Map<String, Object>) methodCall.arguments;
                PlayerConfig config = getConfig();
                if (config != null) {
                    String configJson = mGson.toJson(setConfigMap);
                    config = mGson.fromJson(configJson, PlayerConfig.class);
                    setConfig(config);
                }
            } break;
            case "getConfig":
                PlayerConfig config = getConfig();
                String json = mGson.toJson(config);
                Map<String, Object> configMap = mGson.fromJson(json, Map.class);
                result.success(configMap);
                break;
            case "getCacheConfig":
                CacheConfig cacheConfig = getCacheConfig();
                String cacheConfigJson = mGson.toJson(cacheConfig);
                Map<String, Object> cacheConfigMap = mGson.fromJson(cacheConfigJson, Map.class);
                result.success(cacheConfigMap);
                break;
            case "setCacheConfig":
                Map<String, Object> setCacheConnfigMap = (Map<String, Object>) methodCall.arguments;
                String setCacheConfigJson = mGson.toJson(setCacheConnfigMap);
                CacheConfig setCacheConfig = mGson.fromJson(setCacheConfigJson, CacheConfig.class);
                setCacheConfig(setCacheConfig);
                break;
            case "getCurrentTrack":
                Integer currentTrackIndex = (Integer) methodCall.arguments;
                TrackInfo currentTrack = getCurrentTrack(currentTrackIndex);
                if (currentTrack != null) {
                    Map<String, Object> map = new HashMap<>();
                    //                    map.put("vodFormat",currentTrack.getVodFormat());
                    map.put("videoHeight", currentTrack.getVideoHeight());
                    map.put("videoWidth", currentTrack.getVideoHeight());
                    map.put("subtitleLanguage", currentTrack.getSubtitleLang());
                    map.put("trackBitrate", currentTrack.getVideoBitrate());
                    //                    map.put("vodFileSize",currentTrack.getVodFileSize());
                    map.put("trackIndex", currentTrack.getIndex());
                    //                    map.put("trackDefinition",currentTrack.getVodDefinition());
                    map.put("audioSampleFormat", currentTrack.getAudioSampleFormat());
                    map.put("audioLanguage", currentTrack.getAudioLang());
                    //                    map.put("vodPlayUrl",currentTrack.getVodPlayUrl());
                    map.put("trackType", currentTrack.getType().ordinal());
                    map.put("audioSamplerate", currentTrack.getAudioSampleRate());
                    map.put("audioChannels", currentTrack.getAudioChannels());
                    result.success(map);
                }
                break;
            case "selectTrack":
                Map<String, Object> selectTrackMap = (Map<String, Object>) methodCall.arguments;
                Integer trackIdx = (Integer) selectTrackMap.get("trackIdx");
                Integer accurate = (Integer) selectTrackMap.get("accurate");
                selectTrack(trackIdx, accurate == 1);
                break;
            case "addExtSubtitle":
                String extSubtitlUrl = (String) methodCall.arguments;
                addExtSubtitle(extSubtitlUrl);
                break;
            case "selectExtSubtitle":
                Map<String, Object> selectExtSubtitleMap = (Map<String, Object>) methodCall.arguments;
                Integer trackIndex = (Integer) selectExtSubtitleMap.get("trackIndex");
                Boolean selectExtSubtitlEnable = (Boolean) selectExtSubtitleMap.get("enable");
                selectExtSubtitle(trackIndex, selectExtSubtitlEnable);
                result.success(null);
                break;
            case "enableConsoleLog":
                Boolean enableLog = (Boolean) methodCall.arguments;
                enableConsoleLog(enableLog);
                break;
            case "setLogLevel":
                Integer level = (Integer) methodCall.arguments;
                setLogLevel(level);
                break;
            case "getLogLevel":
                result.success(getLogLevel());
                break;
            case "createDeviceInfo":
                result.success(createDeviceInfo());
                break;
            case "addBlackDevice":
                Map<String, String> addBlackDeviceMap = methodCall.arguments();
                String blackType = addBlackDeviceMap.get("black_type");
                String blackDevice = addBlackDeviceMap.get("black_device");
                addBlackDevice(blackType, blackDevice);
                break;
                //            case "createThumbnailHelper":
                //                String thhumbnailUrl = (String) methodCall.arguments;
                //                createThumbnailHelper(thhumbnailUrl);
                //                break;
                //            case "requestBitmapAtPosition":
                //                Integer requestBitmapProgress = (Integer) methodCall.arguments;
                //                requestBitmapAtPosition(requestBitmapProgress);
                //                break;
            case "getSDKVersion":
                result.success(CicadaPlayerFactory.getSdkVersion());
                break;
                //            case "setPreferPlayerName":
                //                String playerName = methodCall.arguments();
                //                setPlayerName(playerName);
                //                break;
                //            case "getPlayerName":
                //                result.success(getPlayerName());
                //                break;
            case "setStreamDelayTime":
                Map<String, Object> streamDelayTimeMap = (Map<String, Object>) methodCall.arguments;
                Integer index = (Integer) streamDelayTimeMap.get("index");
                Integer time = (Integer) streamDelayTimeMap.get("time");
                setStreamDelayTime(index, time);
                break;
            default:
                result.notImplemented();
        }
    }

    private void createCicadaPlayer()
    {
        mCicadaPlayer = CicadaPlayerFactory.createCicadaPlayer(mContext);
        initListener(mCicadaPlayer);
    }

    private void setDataSource(String url)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setDataSource(url);
        }
    }

    private void prepare()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.prepare();
        }
    }

    private void start()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.start();
        }
    }

    private void pause()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.pause();
        }
    }

    private void stop()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.stop();
        }
    }

    private void release()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.release();
            mCicadaPlayer = null;
        }
    }

    private void seekTo(long position, int seekMode)
    {
        if (mCicadaPlayer != null) {
            CicadaPlayer.SeekMode mSeekMode;
            if (seekMode == CicadaPlayer.SeekMode.Accurate.getValue()) {
                mSeekMode = CicadaPlayer.SeekMode.Accurate;
            } else {
                mSeekMode = CicadaPlayer.SeekMode.Inaccurate;
            }
            mCicadaPlayer.seekTo(position, mSeekMode);
        }
    }

    private MediaInfo getMediaInfo()
    {
        if (mCicadaPlayer != null) {
            return mCicadaPlayer.getMediaInfo();
        }
        return null;
    }

    private void snapshot()
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.snapshot();
        }
    }

    private void setLoop(Boolean isLoop)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setLoop(isLoop);
        }
    }

    private Boolean isLoop()
    {
        return mCicadaPlayer != null && mCicadaPlayer.isLoop();
    }

    private void setAutoPlay(Boolean isAutoPlay)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setAutoPlay(isAutoPlay);
        }
    }

    private Boolean isAutoPlay()
    {
        if (mCicadaPlayer != null) {
            return mCicadaPlayer.isAutoPlay();
        }
        return false;
    }

    private void setMuted(Boolean muted)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setMute(muted);
        }
    }

    private Boolean isMuted()
    {
        if (mCicadaPlayer != null) {
            return mCicadaPlayer.isMute();
        }
        return false;
    }

    private void setEnableHardWareDecoder(Boolean mEnableHardwareDecoder)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.enableHardwareDecoder(mEnableHardwareDecoder);
        }
    }

    private void setScaleMode(int model)
    {
        if (mCicadaPlayer != null) {
            CicadaPlayer.ScaleMode mScaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
            if (model == CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT.getValue()) {
                mScaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
            } else if (model == CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL.getValue()) {
                mScaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL;
            } else if (model == CicadaPlayer.ScaleMode.SCALE_TO_FILL.getValue()) {
                mScaleMode = CicadaPlayer.ScaleMode.SCALE_TO_FILL;
            }
            mCicadaPlayer.setScaleMode(mScaleMode);
        }
    }

    private int getScaleMode()
    {
        int scaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT.getValue();
        if (mCicadaPlayer != null) {
            scaleMode = mCicadaPlayer.getScaleMode().getValue();
        }
        return scaleMode;
    }

    private void setMirrorMode(int mirrorMode)
    {
        if (mCicadaPlayer != null) {
            CicadaPlayer.MirrorMode mMirrorMode;
            if (mirrorMode == CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL.getValue()) {
                mMirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL;
            } else if (mirrorMode == CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL.getValue()) {
                mMirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL;
            } else {
                mMirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
            }
            mCicadaPlayer.setMirrorMode(mMirrorMode);
        }
    }

    private int getMirrorMode()
    {
        int mirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_NONE.getValue();
        if (mCicadaPlayer != null) {
            mirrorMode = mCicadaPlayer.getMirrorMode().getValue();
        }
        return mirrorMode;
    }

    private void setRotateMode(int rotateMode)
    {
        if (mCicadaPlayer != null) {
            CicadaPlayer.RotateMode mRotateMode;
            if (rotateMode == CicadaPlayer.RotateMode.ROTATE_90.getValue()) {
                mRotateMode = CicadaPlayer.RotateMode.ROTATE_90;
            } else if (rotateMode == CicadaPlayer.RotateMode.ROTATE_180.getValue()) {
                mRotateMode = CicadaPlayer.RotateMode.ROTATE_180;
            } else if (rotateMode == CicadaPlayer.RotateMode.ROTATE_270.getValue()) {
                mRotateMode = CicadaPlayer.RotateMode.ROTATE_270;
            } else {
                mRotateMode = CicadaPlayer.RotateMode.ROTATE_0;
            }
            mCicadaPlayer.setRotateMode(mRotateMode);
        }
    }

    private int getRotateMode()
    {
        int rotateMode = CicadaPlayer.RotateMode.ROTATE_0.getValue();
        if (mCicadaPlayer != null) {
            rotateMode = mCicadaPlayer.getRotateMode().getValue();
        }
        return rotateMode;
    }

    private void setSpeed(double speed)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setSpeed((float) speed);
        }
    }

    private double getSpeed()
    {
        double speed = 0;
        if (mCicadaPlayer != null) {
            speed = mCicadaPlayer.getSpeed();
        }
        return speed;
    }

    private void setVideoBackgroundColor(long color)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setVideoBackgroundColor((int) color);
        }
    }

    private void setVolume(double volume)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setVolume((float) volume);
        }
    }

    private double getVolume()
    {
        double volume = 1.0;
        if (mCicadaPlayer != null) {
            volume = mCicadaPlayer.getVolume();
        }
        return volume;
    }

    private void setConfig(PlayerConfig playerConfig)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setConfig(playerConfig);
        }
    }

    private PlayerConfig getConfig()
    {
        if (mCicadaPlayer != null) {
            return mCicadaPlayer.getConfig();
        }
        return null;
    }

    private CacheConfig getCacheConfig()
    {
        return new CacheConfig();
    }

    private void setCacheConfig(CacheConfig cacheConfig)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setCacheConfig(cacheConfig);
        }
    }

    private TrackInfo getCurrentTrack(int currentTrackIndex)
    {
        if (mCicadaPlayer != null) {
            return mCicadaPlayer.currentTrack(currentTrackIndex);
        } else {
            return null;
        }
    }

    private void selectTrack(int trackId, boolean accurate)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.selectTrack(trackId);
        }
    }

    private void addExtSubtitle(String url)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.addExtSubtitle(url);
        }
    }

    private void selectExtSubtitle(int trackIndex, boolean enable)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.selectExtSubtitle(trackIndex, enable);
        }
    }

    private void enableConsoleLog(Boolean enableLog)
    {
        Logger.getInstance(mContext).enableConsoleLog(enableLog);
    }

    private void setLogLevel(int level)
    {
        Logger.LogLevel mLogLevel;
        if (level == Logger.LogLevel.AF_LOG_LEVEL_NONE.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_NONE;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_FATAL.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_FATAL;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_ERROR.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_ERROR;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_WARNING.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_WARNING;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_INFO.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_INFO;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_DEBUG.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_DEBUG;
        } else if (level == Logger.LogLevel.AF_LOG_LEVEL_TRACE.getValue()) {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_TRACE;
        } else {
            mLogLevel = Logger.LogLevel.AF_LOG_LEVEL_NONE;
        }
        Logger.getInstance(mContext).setLogLevel(mLogLevel);
    }

    private Integer getLogLevel()
    {
        return Logger.getInstance(mContext).getLogLevel().getValue();
    }

    private String createDeviceInfo()
    {
        CicadaPlayerFactory.DeviceInfo deviceInfo = new CicadaPlayerFactory.DeviceInfo();
        deviceInfo.model = Build.MODEL;
        return deviceInfo.model;
    }

    private void addBlackDevice(String blackType, String modelInfo)
    {
        CicadaPlayerFactory.DeviceInfo deviceInfo = new CicadaPlayerFactory.DeviceInfo();
        deviceInfo.model = modelInfo;
        CicadaPlayerFactory.BlackType aliPlayerBlackType;
        if (!TextUtils.isEmpty(blackType) && "HW_Decode_H264".equals(blackType)) {
            aliPlayerBlackType = CicadaPlayerFactory.BlackType.HW_Decode_H264;
        } else {
            aliPlayerBlackType = CicadaPlayerFactory.BlackType.HW_Decode_HEVC;
        }
        CicadaPlayerFactory.addBlackDevice(aliPlayerBlackType, deviceInfo);
    }

    //    private void setPlayerName(String playerName) {
    //        if(mCicadaPlayer != null){
    //            mCicadaPlayer.setPreferPlayerName(playerName);
    //        }
    //    }
    //
    //    private String getPlayerName(){
    //        return mCicadaPlayer == null ? "" : mCicadaPlayer.getPlayerName();
    //    }

    private void setStreamDelayTime(int index, int time)
    {
        if (mCicadaPlayer != null) {
            mCicadaPlayer.setStreamDelayTime(index, time);
        }
    }
}
