package com.cicada.player.demo.adapter;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.cicada.player.demo.R;
import com.cicada.player.demo.listener.OnItemClickListener;

import java.util.List;

/**
 * 播放资源列表title  Adapter
 */
public class MediaInfoListTitleAdapter extends RecyclerView.Adapter<MediaInfoListTitleAdapter.MediaInfoViewHolder> {


    private OnItemClickListener itemClickListener;
    private List<String> typeName;

    public MediaInfoListTitleAdapter(List<String> list) {
        this.typeName = list;
    }


    @Override
    public MediaInfoViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_recyclerview_mediainfo_title,
                parent, false);
        return new MediaInfoViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final MediaInfoViewHolder holder, int position) {
        holder.tvTitle.setText(typeName.get(position));
        holder.tvTitle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(itemClickListener != null){
                    itemClickListener.onItemClick(holder.getAdapterPosition());
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        return typeName == null ? 0 : typeName.size();
    }

    public class MediaInfoViewHolder extends RecyclerView.ViewHolder {

        TextView tvTitle;

        public MediaInfoViewHolder(View itemView) {
            super(itemView);
            tvTitle = itemView.findViewById(R.id.tv_title);
        }
    }

    public void setOnItemClickListener(OnItemClickListener itemClickListener){
        this.itemClickListener = itemClickListener;
    }
}
