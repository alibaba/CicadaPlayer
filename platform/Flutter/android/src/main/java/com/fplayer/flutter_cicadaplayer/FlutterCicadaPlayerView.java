package com.fplayer.flutter_cicadaplayer;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import com.cicada.player.CicadaPlayer;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.plugin.common.StandardMessageCodec;
import io.flutter.plugin.platform.PlatformView;
import io.flutter.plugin.platform.PlatformViewFactory;


public class FlutterCicadaPlayerView extends PlatformViewFactory implements PlatformView {


    private TextureView mTextureView;
    private Context mContext;
    private CicadaPlayer mPlayer;


    FlutterCicadaPlayerView(FlutterPlugin.FlutterPluginBinding flutterPluginBinding)
    {
        super(StandardMessageCodec.INSTANCE);
        this.mContext = flutterPluginBinding.getApplicationContext();
    }

    public void setPlayer(CicadaPlayer player)
    {
        this.mPlayer = player;
    }


    @Override public View getView()
    {
        return mTextureView;
    }

    @Override public void dispose()
    {}

    @Override public PlatformView create(Context context, int viewId, Object args)
    {
        mTextureView = new TextureView(mContext);
        initRenderView();
        return this;
    }

    private void initRenderView()
    {
        final TextureView mTextureView;
        mTextureView = (TextureView) getView();
        if (mTextureView != null) {
            mTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
                @Override public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height)
                {
                    Surface mSurface = new Surface(surface);
                    if (mPlayer != null) {
                        mPlayer.setSurface(mSurface);
                    }
                }

                @Override public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height)
                {
                    if (mPlayer != null) {
                        //                        mPlayer.surfaceChanged();
                        mPlayer.redraw();
                    }
                }

                @Override public boolean onSurfaceTextureDestroyed(SurfaceTexture surface)
                {
                    if (mPlayer != null) {
                        mPlayer.setSurface(null);
                    }
                    return false;
                }

                @Override public void onSurfaceTextureUpdated(SurfaceTexture surface)
                {}
            });
        }
    }
}