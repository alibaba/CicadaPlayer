package com.cicada.player.demo.bean;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

import com.google.gson.annotations.SerializedName;

import java.util.ArrayList;

/**
 * @author Mulberry create on 2018/5/17.
 */

public class CicadaVideoInfo implements Parcelable{

    @SerializedName("videoList")
    private ArrayList<Video> videoList;
    @SerializedName("total")
    private String total;

    public CicadaVideoInfo(){

    }

    protected CicadaVideoInfo(Parcel in) {
        total = in.readString();
    }

    public static final Creator<CicadaVideoInfo> CREATOR = new Creator<CicadaVideoInfo>() {
        @Override
        public CicadaVideoInfo createFromParcel(Parcel in) {
            return new CicadaVideoInfo(in);
        }

        @Override
        public CicadaVideoInfo[] newArray(int size) {
            return new CicadaVideoInfo[size];
        }
    };

    public ArrayList<Video> getVideoList() {
        return videoList;
    }

    public void setVideoList(ArrayList<Video> videoList) {
        this.videoList = videoList;
    }

    public String getTotal() {
        return total;
    }

    public void setTotal(String total) {
        this.total = total;
    }

    @Override
    public String toString() {
        return "ClassPojo [VideoList = " + videoList + ", Total = " + total + "]";
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(total);
    }

    public static class Video implements Parcelable {
        @SerializedName("creationTime")
        private String creationTime;
        @SerializedName("coverUrl")
        private String coverURL;
        @SerializedName("status")
        private String status;
        @SerializedName("duration")
        private String duration;
        @SerializedName("title")
        private String title;
        @SerializedName("size")
        private String size;
        @SerializedName("description")
        private String description;
        @SerializedName("cateName")
        private String cateName;
        @SerializedName("cateId")
        private String cateId;
        private String fileUrl;
        private String firstFrameUrl;

        public Video(){

        }


        protected Video(Parcel in) {
            creationTime = in.readString();
            coverURL = in.readString();
            status = in.readString();
            duration = in.readString();
            title = in.readString();
            size = in.readString();
            description = in.readString();
            cateName = in.readString();
            cateId = in.readString();
            fileUrl = in.readString();
        }

        public static final Creator<Video> CREATOR = new Creator<Video>() {
            @Override
            public Video createFromParcel(Parcel in) {
                return new Video(in);
            }

            @Override
            public Video[] newArray(int size) {
                return new Video[size];
            }
        };

        public void setFirstFrameUrl(String firstFrameUrl) {
            this.firstFrameUrl = firstFrameUrl;
        }

        public String getCateId() {
            return cateId;
        }

        public void setCateId(String cateId) {
            this.cateId = cateId;
        }

        public String getCateName() {
            return cateName;
        }

        public void setCateName(String cateName) {
            this.cateName = cateName;
        }

        public String getDescription() {
            return description;
        }

        public void setDescription(String description) {
            this.description = description;
        }

        public String getCoverURL() {
            return coverURL;
        }

        public void setCoverURL(String coverURL) {
            this.coverURL = coverURL;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public String getDuration() {
            return duration;
        }

        public void setDuration(String duration) {
            this.duration = duration;
        }

        public String getCreateTime() {
            return creationTime;
        }

        public void setCreateTime(String creationTime) {
            this.creationTime = creationTime;
        }

        public String getTitle() {
            return title;
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public String getSize() {
            return size;
        }

        public void setSize(String size) {
            this.size = size;
        }

        public String getFileUrl() {
            return fileUrl;
        }

        public void setFileUrl(String fileUrl) {
            this.fileUrl = fileUrl;
        }


        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(creationTime);
            dest.writeString(coverURL);
            dest.writeString(status);
            dest.writeString(duration);
            dest.writeString(title);
            dest.writeString(size);
            dest.writeString(description);
            dest.writeString(cateName);
            dest.writeString(cateId);
            dest.writeString(fileUrl);
        }


        public String getFirstFrameUrl() {
            if(TextUtils.isEmpty(firstFrameUrl)){
                return getCoverURL();
            }else{
                return firstFrameUrl;
            }
        }
    }

    public class Snapshots {
        @SerializedName("Snapshot")
        private String[] snapshot;

        public String[] getSnapshot() {
            return snapshot;
        }

        public void setSnapshot(String[] snapshot) {
            this.snapshot = snapshot;
        }

        @Override
        public String toString() {
            return "ClassPojo [Snapshot = " + snapshot + "]";
        }
    }
}
