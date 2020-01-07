package com.cicada.player.demo;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import com.cicada.player.demo.adapter.MediaInfoChooseListAdapter;
import com.cicada.player.demo.bean.PlayerMediaInfo;
import com.cicada.player.demo.listener.OnItemClickListener;
import com.cicada.player.demo.multiInstance.MultiPlayerActivity;
import com.cicada.player.demo.util.FileUtils;
import com.cicada.player.demo.util.SourceListParser;
import com.cicada.player.demo.util.ThreadUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * 选择资源列表界面
 */
public class SourceChooseListActivity extends BaseActivity implements OnItemClickListener, View.OnClickListener {

    /**
     * 播放方式
     */
    private String typeName;
    /**
     * 不同的播放方式下的具体的播放资源集合
     */
    private ArrayList<PlayerMediaInfo.TypeInfo> typeInfos = new ArrayList<>();
    private RecyclerView recyclerView;
    private MediaInfoChooseListAdapter mediaInfoChooseListAdapter;
    private TextView tvRight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_source_choose_list);

        typeName = getIntent().getStringExtra("typeName");

        initView();
        initRecyclerView();
        initData();
    }

    private void initView() {
        TextView tvTitle = findViewById(R.id.tv_title);
        recyclerView = findViewById(R.id.recycler_source_list);
//        tvTitle.setText(typeName);
        tvTitle.setText(String.format(Locale.ENGLISH,"%s",typeName));

        tvRight = findViewById(R.id.tv_right);
        tvRight.setText(getString(R.string.cicada_multiple_instances));
    }

    private void initRecyclerView() {
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false);
        recyclerView.setLayoutManager(linearLayoutManager);
    }

    private void initData() {
        //在子线程中加载相关数据
        ThreadUtils.runOnSubThread(new Runnable() {
            @Override
            public void run() {

                /**
                 * 播放资源map集合,key为播放方式,value为具体的播放资源
                 */
                Map<String, ArrayList<PlayerMediaInfo.TypeInfo>> mediaInfoMap = null;

                List<PlayerMediaInfo> mediaInfos = SourceListParser.parse(getApplicationContext());
                if (mediaInfos != null) {
                    mediaInfoMap = SourceListParser.handleDate(mediaInfos);
                }

                if (mediaInfoMap != null && !mediaInfoMap.isEmpty()) {
                    for(String key: mediaInfoMap.keySet()){
                        if(typeName.equals(key)){
                            typeInfos.addAll(mediaInfoMap.get(key));
                        }
                    }
                }

                if (typeInfos.isEmpty()) {
                    //本地视频
                    File file = new File(Environment.getExternalStorageDirectory().getAbsolutePath());
                    typeInfos = FileUtils.getPathList(file);
                }

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mediaInfoChooseListAdapter = new MediaInfoChooseListAdapter(typeInfos);
                        recyclerView.setAdapter(mediaInfoChooseListAdapter);
                        initListener();
                    }
                });
            }
        });
    }

    private void initListener() {
        if (mediaInfoChooseListAdapter != null) {
            mediaInfoChooseListAdapter.setOnItemClickListener(this);
        }
        tvRight.setOnClickListener(this);

    }

    /**
     * 列表 Item 点击监听
     */
    @Override
    public void onItemClick(int position) {

        if (typeInfos == null) {

        } else {
            PlayerMediaInfo.TypeInfo typeInfo = typeInfos.get(position);
            //URL
            if (CicadaPlayerActivity.URL_TYPE.equalsIgnoreCase(typeInfo.getType())
                    || CicadaPlayerActivity.LIVE_TYPE.equalsIgnoreCase(typeInfo.getType())) {
                if(typeInfo.getSubtitle() != null &&
                        (!TextUtils.isEmpty(typeInfo.getSubtitle().getCn()) || (!TextUtils.isEmpty(typeInfo.getSubtitle().getEn())))){
                    CicadaPlayerActivity.startApsaraPlayerActivityByUrlWithSubtitle(this, typeInfo);
                }else{
                    CicadaPlayerActivity.startApsaraPlayerActivityByUrl(this, typeInfo.getUrl());
                }

            }
        }
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tv_right:
                //多实例播放
                for (PlayerMediaInfo.TypeInfo typeInfo : typeInfos) {
                    if (typeInfo.isChecked()) {
                        MultiPlayerActivity.mMultiPlayerBeanList.add(typeInfo);
                    }
                }
                startActivity(MultiPlayerActivity.class);
                break;
        }
    }
}
