package com.cicada.player.demo;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.text.Editable;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.cicada.player.zxing.activity.CaptureActivity;

/**
 * 播放Url输入界面
 */
public class SourceInputUrlActivity extends BaseActivity implements View.OnClickListener {

    private static final int REQ_CODE_PERMISSION = 0x1111;

    /**
     * 进入阿里云播放器播放
     */
    private Button mEnterAliyunPlayerButton;

    /**
     * 进入exoPlayer
     */
    private Button mEnterExoPlayerButton;
    private EditText mInputUrlEditText;
    private TextView mTitleRightTextView;

    public static void startSourceInputUrlActivity(Context context) {
        Intent intent = new Intent(context, SourceInputUrlActivity.class);
        context.startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_source_input_url);

        initView();
        initListener();
    }

    private void initView() {
        TextView tvTitle = findViewById(R.id.tv_title);
        mTitleRightTextView = findViewById(R.id.tv_right);
        mInputUrlEditText = findViewById(R.id.et_input_url);
        mEnterExoPlayerButton = findViewById(R.id.btn_enter_exoplayer);
        mEnterAliyunPlayerButton = findViewById(R.id.btn_enter_cicada_player);

        tvTitle.setText(R.string.title_input_url_item);
        mTitleRightTextView.setText(R.string.title_scanning);

    }

    private void initListener() {
        mTitleRightTextView.setOnClickListener(this);
        mEnterExoPlayerButton.setOnClickListener(this);
        mEnterAliyunPlayerButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Editable editable = mInputUrlEditText.getText();
        String url = "";
        switch (v.getId()) {
            //进入URL播放
            case R.id.btn_enter_cicada_player:

                if (editable != null) {
                    url = mInputUrlEditText.getText().toString();
                }
                CicadaPlayerActivity.startApsaraPlayerActivityByUrl(this, url);
                break;
            case R.id.btn_enter_exoplayer:
                //exoPlayer
                if (editable != null) {
                    url = mInputUrlEditText.getText().toString();
                }
                if (TextUtils.isEmpty(url)) {
                    CharSequence hint = mInputUrlEditText.getHint();
                    if (hint != null) {
                        url = hint.toString();
                    }
                }
                ExoPlayerActivity.startExoPlayerActivity(this, url);
                break;
            case R.id.tv_right:
                //二维码扫描
                if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
                    // Do not have the permission of camera, request it.
                    ActivityCompat.requestPermissions(this,
                            new String[] {Manifest.permission.CAMERA}, REQ_CODE_PERMISSION);
                } else {
                    // Have gotten the permission
                    startCaptureActivityForResult();
                }
                break;
            default:
                break;
        }
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mInputUrlEditText != null) {
            mInputUrlEditText = null;
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQ_CODE_PERMISSION: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // User agree the permission
                    startCaptureActivityForResult();
                } else {
                    // User disagree the permission
                    Toast.makeText(this, "You must agree the camera permission request before you use the code scan function", Toast.LENGTH_SHORT).show();
                }
            }
            break;
            default:
                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == CaptureActivity.REQ_CODE){
            if(resultCode == RESULT_OK){
                mInputUrlEditText.setText(data.getStringExtra(CaptureActivity.INTENT_EXTRA_KEY_QR_SCAN));
            }
        }
    }

    private void startCaptureActivityForResult(){
        Intent intent = new Intent(this, CaptureActivity.class);
        startActivityForResult(intent, CaptureActivity.REQ_CODE);
    }
}
