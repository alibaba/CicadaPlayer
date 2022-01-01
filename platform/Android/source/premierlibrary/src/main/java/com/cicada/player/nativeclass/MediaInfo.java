package com.cicada.player.nativeclass;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class MediaInfo {

    private List<TrackInfo> mTrackInfos = new ArrayList<TrackInfo>();

    /**
     * 获取媒体所有的流信息
     *
     * @return 流信息。见{@link TrackInfo}
     */
    /****
     * Query all stream information of the media.
     *
     * @return The stream information. See {@link TrackInfo}.
     */
    public List<TrackInfo> getTrackInfos() {
        return mTrackInfos;
    }


    public void setTrackInfos(TrackInfo[] trackInfos) {
        if (trackInfos != null && trackInfos.length > 0) {
            mTrackInfos.addAll(Arrays.asList(trackInfos));
        }
    }


    private Object[] getTrackInfoArray() {
        return mTrackInfos.toArray();
    }
}
