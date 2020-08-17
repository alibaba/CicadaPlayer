package com.aliyun.externalplayer.exo;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.TextureView;

import com.cicada.player.CicadaPlayer;

public class ExternExoTextureView extends TextureView {

    private int mVideoWidth;
    private int mVideoHeight;

    private CicadaPlayer.ScaleMode mCurrentAspectRatio = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;

    public ExternExoTextureView(@NonNull Context context) {
        super(context);
    }

    public ExternExoTextureView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public ExternExoTextureView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public void setVideoSize(int width,int height){
        this.mVideoWidth = width;
        this.mVideoHeight = height;
    }

    public void setScaleType(CicadaPlayer.ScaleMode scaleType){
        this.mCurrentAspectRatio = scaleType;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mVideoHeight == 0 || mVideoWidth == 0) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            return;
        }

        if (getRotation() == 90 || getRotation() == 270) {
            int tempSpec = widthMeasureSpec;
            widthMeasureSpec = heightMeasureSpec;
            heightMeasureSpec = tempSpec;
        }

        int width;
        int height;

        int measuredWith = MeasureSpec.getSize(widthMeasureSpec);
        int measuredHeight = MeasureSpec.getSize(heightMeasureSpec);

        if (mCurrentAspectRatio == CicadaPlayer.ScaleMode.SCALE_TO_FILL) {
            width = measuredWith;
            height = measuredHeight;
        }else if(mCurrentAspectRatio == CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL){

            if(mVideoWidth > mVideoHeight){
                height = Math.max(heightMeasureSpec,mVideoHeight);
                width = height * mVideoWidth / mVideoHeight;
            }else{
                width = Math.max(measuredWith,mVideoWidth);
                height = width * mVideoHeight / mVideoWidth;
            }
        }else{
            width = measuredWith;
            height = measuredHeight;

            if (mVideoWidth * height < width * mVideoHeight) {
                width = height * mVideoWidth / mVideoHeight;
            } else if (mVideoWidth * height > width * mVideoHeight) {
                height = width * mVideoHeight / mVideoWidth;
            }
        }

        setMeasuredDimension(width, height);
    }

}
