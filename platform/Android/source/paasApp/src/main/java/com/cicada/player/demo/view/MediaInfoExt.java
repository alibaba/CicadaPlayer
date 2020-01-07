package com.cicada.player.demo.view;

import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.TrackInfo;

import java.util.List;

public class MediaInfoExt {

    private MediaInfo mediaInfo;
    private int duration;

    public MediaInfoExt(MediaInfo mediaInfo) {
        this.mediaInfo = mediaInfo;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public List<TrackInfo> getTrackInfos() {
        if (mediaInfo != null) {
            return mediaInfo.getTrackInfos();
        } else {
            return null;
        }
    }

}
