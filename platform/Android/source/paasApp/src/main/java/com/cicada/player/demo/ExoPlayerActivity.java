package com.cicada.player.demo;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;
import android.widget.Button;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultLoadControl;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.dash.DashMediaSource;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.source.smoothstreaming.SsMediaSource;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.ui.PlayerView;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.util.Util;

public class ExoPlayerActivity extends BaseActivity implements View.OnClickListener {

    private static final String DATA_SOURCE_URL = "data_source_url";
    /**
     * 准备
     */
    private Button mPrepareButton;
    /**
     * ExoPlyaerView
     */
    private PlayerView mExoPlayerView;
    private SimpleExoPlayer mExoPlayer;
    private String mPlayUrl;
    private MediaSource mMediaSource;
    private DataSource.Factory dataSourceFactory;

    public static void startExoPlayerActivity(Context context, String url) {
        Intent intent = new Intent(context, ExoPlayerActivity.class);
        intent.putExtra(DATA_SOURCE_URL, url);
        context.startActivity(intent);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_exoplayer);

        mPlayUrl = getIntent().getStringExtra(DATA_SOURCE_URL);
        initView();
        initListener();
        initPlayer();
    }

    private void initView() {
        mPrepareButton = findViewById(R.id.prepare);
        mExoPlayerView = findViewById(R.id.exo_player_view);
    }

    private void initListener() {
        mPrepareButton.setOnClickListener(this);
    }

    private void initPlayer(){
        mExoPlayer = ExoPlayerFactory.newSimpleInstance(getApplicationContext(),
                new DefaultRenderersFactory(this),
                new DefaultTrackSelector(),
                new DefaultLoadControl());
        mExoPlayerView.requestFocus();
        mExoPlayerView.setPlayer(mExoPlayer);
        mExoPlayer.setPlayWhenReady(true);
        mMediaSource = buildMediaSource();
    }

    private MediaSource buildMediaSource(){
        Uri uri = Uri.parse(mPlayUrl);
        int type = Util.inferContentType(uri);
        DefaultDataSourceFactory defaultDataSourceFactory = new DefaultDataSourceFactory(this,"player");


        MediaSource mediaSource = null;
        switch (type){
            case C.TYPE_DASH:
                mediaSource = new DashMediaSource.Factory(defaultDataSourceFactory).createMediaSource(uri);
                break;
            case C.TYPE_SS:
                mediaSource = new SsMediaSource.Factory(defaultDataSourceFactory).createMediaSource(uri);
                break;
            case C.TYPE_HLS:
                mediaSource = new HlsMediaSource.Factory(defaultDataSourceFactory)
                        .createMediaSource(uri);
                break;
            case C.TYPE_OTHER:
                mediaSource = new ExtractorMediaSource.Factory(defaultDataSourceFactory).createMediaSource(uri);
                break;
        }

        return mediaSource;
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.prepare:
                //准备
                mExoPlayer.prepare(mMediaSource,false,true);
                break;
            default:
                break;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mExoPlayer.stop(true);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mExoPlayer != null){
            mExoPlayer.release();
            mExoPlayer = null;
        }
    }
}
