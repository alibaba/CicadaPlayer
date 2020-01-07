package com.cicada.player.demo.adapter;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.cicada.player.CicadaPlayer;
import com.cicada.player.CicadaPlayerFactory;
import com.cicada.player.demo.R;
import com.cicada.player.demo.bean.PlayerMediaInfo;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 多实例播放器Adapter
 */
public class MultiPlayerAdapter extends RecyclerView.Adapter<MultiPlayerAdapter.MutiPlayerViewHolder> {

    private List<PlayerMediaInfo.TypeInfo> datas;
    private Map<String, CicadaPlayer> map = new HashMap<>();

    public MultiPlayerAdapter(Context context, List<PlayerMediaInfo.TypeInfo> list) {
        this.datas = list;
        for (PlayerMediaInfo.TypeInfo data : datas) {
            CicadaPlayer cicadaPlayer = CicadaPlayerFactory.createCicadaPlayer(context);
            map.put(getSource(data), cicadaPlayer);
        }
    }

    /**
     * 获取vid/url
     */
    public String getSource(PlayerMediaInfo.TypeInfo typeInfo) {
        return typeInfo.getUrl();
    }

    /**
     * 获取CicadaPlayer
     */
    public CicadaPlayer getCicadaPlayer(int position) {
        PlayerMediaInfo.TypeInfo typeInfo = datas.get(position);
        String source = getSource(typeInfo);
        return map.get(source);
    }

    @NonNull
    @Override
    public MutiPlayerViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_recyclerview_multiplayer, parent, false);
        return new MutiPlayerViewHolder(view);
    }


    @Override
    public void onBindViewHolder(@NonNull MutiPlayerViewHolder holder, int position) {

        PlayerMediaInfo.TypeInfo typeInfo = datas.get(position);
        String source = getSource(typeInfo);
        CicadaPlayer cicadaPlayer = map.get(source);

        holder.mTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
                cicadaPlayer.setSurface(null);
                Surface surface = new Surface(surfaceTexture);
                cicadaPlayer.setSurface(surface);
                cicadaPlayer.redraw();
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {

            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
                return false;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

            }
        });

        holder.mPrepareButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cicadaPlayer.setDataSource(source);
                cicadaPlayer.prepare();
            }
        });

        holder.mPlayButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                getCicadaPlayer(position).start();
            }
        });

        holder.mPauseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                getCicadaPlayer(position).pause();
            }
        });

        holder.mStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                getCicadaPlayer(position).stop();
            }
        });
    }

    @Override
    public int getItemCount() {
        return datas == null ? 0 : datas.size();
    }

    public class MutiPlayerViewHolder extends RecyclerView.ViewHolder {

        private Button mPrepareButton;
        private Button mPlayButton;
        private Button mPauseButton;
        private Button mStopButton;
        private TextureView mTextureView;

        public MutiPlayerViewHolder(View itemView) {
            super(itemView);
            mPrepareButton = itemView.findViewById(R.id.btn_prepare);
            mPlayButton = itemView.findViewById(R.id.btn_play);
            mPauseButton = itemView.findViewById(R.id.btn_pause);
            mStopButton = itemView.findViewById(R.id.btn_stop);
            mTextureView = itemView.findViewById(R.id.textureview);
        }
    }

    public void destroy() {
        for (String source : map.keySet()) {
            map.get(source).stop();
            map.get(source).release();
        }
    }
}
