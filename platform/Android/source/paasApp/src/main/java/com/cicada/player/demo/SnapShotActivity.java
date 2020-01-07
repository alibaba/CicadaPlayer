package com.cicada.player.demo;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.ImageView;
import android.widget.TextView;

import java.lang.ref.WeakReference;

/**
 * 截图显示Activity
 */
public class SnapShotActivity extends BaseActivity {

    /**
     * Intent的key
     */
    public static final String SCREEN_SHOT = "screen_shot";
    /**
     * 截图
     */
    private static final int MSG_SCREEN_SHOT = 1;

    /**
     * handler
     */
    private MyHandler mHandler = new MyHandler(this);
    /**
     * 用于展示截图的ImageView
     */
    private ImageView mScreenShotImageView;

    public static void startScreenshotActivity(Context context, String bitmapPath) {
        Intent intent = new Intent(context, SnapShotActivity.class);
        intent.putExtra(SCREEN_SHOT, bitmapPath);
        context.startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_screen_shot);

        initView();
        initData();
    }

    private void initView() {
        TextView mTitleTextView = findViewById(R.id.tv_title);
        mTitleTextView.setText(R.string.title_screen_shot);

        mScreenShotImageView = findViewById(R.id.iv_screen_shot);

    }

    private void initData() {
//        byte[] bis = getIntent().getByteArrayExtra(SCREEN_SHOT);
        String bitmapPath = getIntent().getStringExtra(SCREEN_SHOT);
//        Bitmap bitmap = BitmapFactory.decodeByteArray(bis, 0, bis.length);
        Bitmap bitmap = BitmapFactory.decodeFile(bitmapPath);
        mScreenShotImageView.setImageBitmap(bitmap);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHandler.sendEmptyMessageDelayed(MSG_SCREEN_SHOT, 5000);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mHandler.removeCallbacksAndMessages(null);
        mScreenShotImageView = null;
    }

    private static class MyHandler extends Handler {

        private WeakReference<SnapShotActivity> weakReference;

        public MyHandler(SnapShotActivity activity) {
            weakReference = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_SCREEN_SHOT:
                    SnapShotActivity snapShotActivity = weakReference.get();
                    if (snapShotActivity != null) {
                        snapShotActivity.finish();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
