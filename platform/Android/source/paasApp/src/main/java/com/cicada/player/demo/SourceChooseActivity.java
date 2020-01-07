package com.cicada.player.demo;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.cicada.player.demo.adapter.MediaInfoListTitleAdapter;
import com.cicada.player.demo.bean.PlayerMediaInfo;
import com.cicada.player.demo.listener.OnItemClickListener;
import com.cicada.player.demo.util.Common;
import com.cicada.player.demo.util.PermissionUtils;
import com.cicada.player.demo.util.SourceListParser;
import com.cicada.player.demo.util.ThreadUtils;

import java.lang.ref.WeakReference;
import java.util.List;

/**
 * 选择资源Activity
 */
public class SourceChooseActivity extends BaseActivity implements View.OnClickListener, OnItemClickListener {

    /**
     * 权限请求码
     */
    private static final int PERMISSION_REQUEST_CODE = 1001;
    /**
     * 输入URL
     */
    private Button mInputUrlButton;

    /**
     * 该集合用于保存获取到的所有数据的所有播放方式,
     */
    private List<String> typeNameList;
    private RecyclerView recyclerView;
    private MediaInfoListTitleAdapter titleAdapter;

    /**
     * 权限
     */
    String[] permission = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    private FrameLayout mDownloadFragmentLayout;
    private LinearLayout mRootLinearLayout;
    private TextView mRightTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        copyAssets();
        setContentView(R.layout.activity_source_choose);

        initView();
        initRecyclerView();
        initData();
        initListener();
        initFragment();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    /**
     * 拷贝文件
     */
    private void copyAssets() {
        Common commenUtils = Common.getInstance(getApplicationContext()).copyAssetsToSD("encrypt", "aliyunPlayer");
        CustomFileOperateCallback mCustomFileOperateCallback = new CustomFileOperateCallback(this);
        commenUtils.setFileOperateCallback(mCustomFileOperateCallback);
    }


    private void initView() {
        TextView mTitleTextView = findViewById(R.id.tv_title);


        mInputUrlButton = findViewById(R.id.btn_input_url);

        mRootLinearLayout = findViewById(R.id.ll_root);


        recyclerView = findViewById(R.id.recycview_date_list);
        mDownloadFragmentLayout = findViewById(R.id.fl_download);

        mTitleTextView.setText(R.string.title_choose_item);

        mRightTextView = findViewById(R.id.tv_right);
        mRightTextView.setText(R.string.title_setting);
    }

    private void initListener() {

        mRightTextView.setOnClickListener(this);
        mInputUrlButton.setOnClickListener(this);
    }

    private void initFragment() {
    }


    private void initRecyclerView() {
        recyclerView.setNestedScrollingEnabled(true);
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false);
        recyclerView.setLayoutManager(linearLayoutManager);
    }

    private void initData() {
        boolean checkResult = PermissionUtils.checkPermissionsGroup(this, permission);
        if (!checkResult) {
            PermissionUtils.requestPermissions(this, permission, PERMISSION_REQUEST_CODE);
        } else {
            readListData();
        }
    }

    /**
     * 从 txt 中读取数据,并设置到RecyclerView中
     */
    private void readListData() {
        List<PlayerMediaInfo> mediaInfos = SourceListParser.parse(this);
        if (mediaInfos != null) {
            typeNameList = SourceListParser.getDateTitleKey(mediaInfos);
        }
        titleAdapter = new MediaInfoListTitleAdapter(typeNameList);
        recyclerView.setAdapter(titleAdapter);

        titleAdapter.setOnItemClickListener(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean isAllGranted = true;

            // 判断是否所有的权限都已经授予了
            for (int grant : grantResults) {
                if (grant != PackageManager.PERMISSION_GRANTED) {
                    isAllGranted = false;
                    break;
                }
            }

            if (isAllGranted) {
                // 如果所有的权限都授予了
                //从sourceList.txt中读取数据
                readListData();
            } else {
                // 弹出对话框告诉用户需要权限的原因, 并引导用户去应用权限管理中手动打开权限按钮
                showPermissionDialog();
            }
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_input_url:
                //输入URL
                SourceInputUrlActivity.startSourceInputUrlActivity(this);

                break;
            case R.id.tv_right:
                //设置
                startActivity(SettingActivity.class);
                break;
            default:
                break;
        }
    }
    /**
     * RecyclerView item 点击事件
     */
    @Override
    public void onItemClick(int position) {
        Intent intent = new Intent(this, SourceChooseListActivity.class);
        intent.putExtra("typeName", typeNameList.get(position));
        startActivity(intent);
    }


    @Override
    public void onBackPressed() {
        if (mDownloadFragmentLayout.isShown()) {
            //隐藏下载界面
            mRootLinearLayout.setVisibility(View.VISIBLE);
            mDownloadFragmentLayout.setVisibility(View.GONE);
            return;
        }
        super.onBackPressed();
    }

    private static class CustomFileOperateCallback implements Common.FileOperateCallback {

        private WeakReference<SourceChooseActivity> weakReference;

        public CustomFileOperateCallback(SourceChooseActivity sourceChooseActivity) {
            weakReference = new WeakReference<>(sourceChooseActivity);
        }

        @Override
        public void onSuccess() {
        }

        @Override
        public void onFailed(String error) {
            ThreadUtils.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    SourceChooseActivity sourceChooseActivity = weakReference.get();
                    if (sourceChooseActivity != null) {
                        Toast.makeText(sourceChooseActivity, sourceChooseActivity.getString(R.string.cicada_copy_file_failure), Toast.LENGTH_SHORT).show();
                    }
                }
            });
        }
    }
}
