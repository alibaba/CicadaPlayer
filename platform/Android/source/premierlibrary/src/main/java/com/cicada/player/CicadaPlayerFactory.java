package com.cicada.player;

import android.content.Context;
import android.os.Build;

import com.cicada.player.nativeclass.NativeExternalPlayer;
import com.cicada.player.utils.ContentDataSource;
import com.cicada.player.externalplayer.MediaPlayer;
import com.cicada.player.nativeclass.NativePlayerBase;

import java.util.ArrayList;
import java.util.List;

import static android.os.Build.VERSION_CODES.JELLY_BEAN_MR2;


/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class CicadaPlayerFactory {

    private static List<DeviceInfo> sInnerBlackList = new ArrayList<DeviceInfo>();

    static {
        initInnerBlackList();
        addBlackList(BlackType.HW_Decode_H264, sInnerBlackList);
        CicadaExternalPlayer.registerExternalPlayer(new MediaPlayer());
    }

    private static void initInnerBlackList() {
        DeviceInfo deviceInfo = new DeviceInfo();
        deviceInfo.model="Lenovo K320t";
        sInnerBlackList.add(deviceInfo);
    }

    /**
     * 创建播放器
     *
     * @param context 上下文。
     * @return 播放器对象。
     */
    /****
     * Create a player.
     *
     * @param context The context.
     * @return The player object.
     */
    public static CicadaPlayer createCicadaPlayer(Context context) {
        return createCicadaPlayer(context, null);
    }

    /**
     * 创建播放器
     *
     * @param context 上下文。
     * @param name 外部播放器的名字
     * @return 播放器对象。
     */
    /****
     * Create a player.
     *
     * @param context The context.
     * @param name external player name
     * @return The player object.
     */
    public static CicadaPlayer createCicadaPlayer(Context context , String name) {
        return createCicadaPlayer(context, name , null);
    }

    /**
     * 创建播放器
     *
     * @param context 上下文。
     * @param traceId 便于跟踪日志。
     * @return 播放器对象。
     */
    /****
     * Create a player.
     *
     * @param context The context.
     * @param traceId A trace ID for troubleshooting with the relevant log.
     * @return The player object.
     */
    public static CicadaPlayerImpl createCicadaPlayer(Context context, String name, String traceId) {
        ContentDataSource.setContext(context);
        NativeExternalPlayer.setContext(context);
        return new CicadaPlayerImpl(context, name , traceId);
    }

    /**
     * 获取SDK版本号
     *
     * @return SDK版本号
     */
    /****
     * Query the version of the SDK.
     *
     * @return The version of the SDK.
     */
    public static String getSdkVersion() {
        return NativePlayerBase.getSdkVersion();
    }

    /**
     * 黑名单类型
     */
    /****
     * The type of the blacklist.
     */
    public static enum BlackType {
        /**
         * h264硬解码
         */
        /****
         * H264 hardware decoding.
         */
        HW_Decode_H264,

        HW_Decode_HEVC
    }

    /**
     * 设备信息
     */
    /****
     * Device information.
     */
    public static class DeviceInfo {
        /**
         * 设备model。获取方式为：{@linkplain Build#MODEL}
         */
        /****
         * The model of the device. To query the device model, see {@linkplain Build#MODEL}.
         */
        public String model;
    }

    /**
     * 添加黑名单
     * @param blackType 黑名单类型。见{@linkplain BlackType}.
     * @param deviceInfo 设备信息。见{@linkplain DeviceInfo}
     */
    /****
     * Add a device to the blacklist.
     * @param blackType The type of the blacklist. See {@linkplain BlackType}.
     * @param deviceInfo Information of the specified device. See {@linkplain DeviceInfo}.
     */
    public static void addBlackDevice(BlackType blackType, DeviceInfo deviceInfo) {
        if (deviceInfo == null || blackType == null) {
            return;
        }

        int    apiLevel    = Build.VERSION.SDK_INT;
        String deviceModel = Build.MODEL;

        if (blackType == BlackType.HW_Decode_H264) {
            if (apiLevel < JELLY_BEAN_MR2) {// not support hard decode
                NativePlayerBase.setBlackType(blackType.ordinal());
                return;
            }

            if (deviceModel.equals(deviceInfo.model)) {// this device is in black list.
                NativePlayerBase.setBlackType(blackType.ordinal());
                return;
            }
        }
    }

    /**
     * 添加一组黑名单
     * @param blackType 黑名单类型。见{@linkplain BlackType}.
     * @param deviceInfos 一组设备信息。见{@linkplain DeviceInfo}
     */
    /****
     * Add multiple devices to the blacklist.
     * @param blackType The type of the blacklist. See {@linkplain BlackType}.
     * @param deviceInfos Information of the specified devices. See {@linkplain DeviceInfo}.
     */
    public static void addBlackList(BlackType blackType, List<DeviceInfo> deviceInfos) {
        if (deviceInfos == null) {
            return;
        }

        for (DeviceInfo deviceInfo : deviceInfos) {
            addBlackDevice(blackType, deviceInfo);
        }
    }

}
