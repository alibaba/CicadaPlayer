package com.aliyun.externalplayer.exo;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.TextureView;

public class ExternExoSurface extends Surface {

    private TextureView mTextureView;

    public ExternExoSurface(SurfaceTexture from) {
        super(from);
    }

    public void setTextureView(TextureView textureView){
        this.mTextureView = textureView;
    }

    public TextureView getTextureView(){
        return mTextureView;
    }
}
