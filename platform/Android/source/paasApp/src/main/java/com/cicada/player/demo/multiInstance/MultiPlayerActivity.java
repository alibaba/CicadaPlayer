package com.cicada.player.demo.multiInstance;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.widget.TextView;

import com.cicada.player.demo.BaseActivity;
import com.cicada.player.demo.R;
import com.cicada.player.demo.adapter.MultiPlayerAdapter;
import com.cicada.player.demo.bean.PlayerMediaInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * 多实例Player
 */
public class MultiPlayerActivity extends BaseActivity {

    public static List<PlayerMediaInfo.TypeInfo> mMultiPlayerBeanList = new ArrayList<>();

    private RecyclerView mRecyclerView;
    private MultiPlayerAdapter mMutiPlayerAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_multi_players);

        initView();
        initRecyclerView();
    }

    private void initView(){
        TextView mTitleTextView = findViewById(R.id.tv_title);
        mTitleTextView.setText(getString(R.string.cicada_multiple_instances));

        mRecyclerView = findViewById(R.id.recyclerview);
    }

    private void initRecyclerView(){
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(this,LinearLayoutManager.VERTICAL,false);
        mRecyclerView.setLayoutManager(linearLayoutManager);

        mMutiPlayerAdapter = new MultiPlayerAdapter(this, MultiPlayerActivity.mMultiPlayerBeanList);
        mRecyclerView.setAdapter(mMutiPlayerAdapter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MultiPlayerActivity.mMultiPlayerBeanList.clear();
        if(mMutiPlayerAdapter != null){
            mMutiPlayerAdapter.destroy();
        }
    }
}
