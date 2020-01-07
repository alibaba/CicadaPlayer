package com.cicada.player.demo.adapter;

import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cicada.player.demo.bean.PlayerMediaInfo;
import com.cicada.player.demo.R;
import com.cicada.player.demo.listener.OnItemClickListener;

import java.util.List;

/**
 * 资源列表地选择Adapter
 */
public class MediaInfoChooseListAdapter extends RecyclerView.Adapter<MediaInfoChooseListAdapter.MediaInfoViewHolder> {

    private List<PlayerMediaInfo.TypeInfo> mediaInfos;

    private OnItemClickListener onItemClickListener;

    public MediaInfoChooseListAdapter(List<PlayerMediaInfo.TypeInfo> list) {
        this.mediaInfos = list;
    }

    @Override
    public MediaInfoViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_recyclerview_mediainfo_list,
                parent, false);
        return new MediaInfoViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final MediaInfoViewHolder holder, int position) {
        final PlayerMediaInfo.TypeInfo typeInfo = mediaInfos.get(position);

        if (!TextUtils.isEmpty(typeInfo.getUrl())) {
            holder.tvListLink.setText(typeInfo.getUrl());
            holder.mSelectCheckBox.setTag(typeInfo.getUrl());
        }

        holder.mNameTextView.setText(mediaInfos.get(position).getName());
        holder.mSelectCheckBox.setChecked(typeInfo.isChecked());

        holder.llRoot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (onItemClickListener != null) {
                    onItemClickListener.onItemClick(holder.getAdapterPosition());
                }
            }
        });

        holder.mSelectCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean checked) {
                if (!TextUtils.isEmpty(typeInfo.getUrl())) {
                    if (holder.mSelectCheckBox.getTag().equals(typeInfo.getUrl())) {
                        typeInfo.setChecked(checked);
                    }
                }
            }
        });


    }

    @Override
    public int getItemCount() {
        return mediaInfos == null ? 0 : mediaInfos.size();
    }

    public class MediaInfoViewHolder extends RecyclerView.ViewHolder {

        private TextView tvListLink;
        private RelativeLayout llRoot;
        private TextView mNameTextView;
        private CheckBox mSelectCheckBox;

        public MediaInfoViewHolder(View itemView) {
            super(itemView);
            llRoot = itemView.findViewById(R.id.ll_root);
            mNameTextView = itemView.findViewById(R.id.tv_name);
            tvListLink = itemView.findViewById(R.id.tv_list_link);
            mSelectCheckBox = itemView.findViewById(R.id.checkbox_select);
        }
    }

    public void setOnItemClickListener(OnItemClickListener listener) {
        this.onItemClickListener = listener;
    }
}
