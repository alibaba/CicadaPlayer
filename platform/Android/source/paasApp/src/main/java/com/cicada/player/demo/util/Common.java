package com.cicada.player.demo.util;

import android.content.Context;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;

import com.cicada.player.demo.bean.CicadaVideoInfo;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 公共目录
 */
public class Common {

    /**
     * 是否已经加入黑名单
     */
    public static boolean HAS_ADD_BLACKLIST = false;

    /**
     * 视频列表缓存目录
     */
    public static String DIR_CACHE = Environment.getExternalStorageDirectory().getAbsolutePath()+"/CicadaVideo/cache";

    private static Common instance;
    private static final int SUCCESS = 1;
    private static final int FAILED = 0;
    private Context context;
    private FileOperateCallback callback;
    private volatile boolean isSuccess;
    private String errorStr;
    private ThreadPoolExecutor threadPoolExecutor;
    private String srcPath;
    private String sdPath;

    public static Common getInstance(Context context) {
        if (instance == null) {
            synchronized (Common.class) {
                if (instance == null) {
                    instance = new Common(context);
                }
            }
        }
        return instance;
    }

    private Common(Context context) {
        this.context = context;
    }

    private Handler handler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (callback != null) {
                if (msg.what == SUCCESS) {
                    callback.onSuccess();
                }
                if (msg.what == FAILED) {
                    callback.onFailed(msg.obj.toString());
                }
            }
            threadPoolExecutor.remove(runnable);
            context = null;
            instance = null;
        }
    };

    public Common copyAssetsToSD(final String srcPath, final String sdPath) {
        this.srcPath = srcPath;
        this.sdPath = sdPath;

        threadPoolExecutor = new ThreadPoolExecutor(1, 1, 0L, TimeUnit.SECONDS, new LinkedBlockingDeque<Runnable>(),
            new ThreadFactory() {
                @Override
                public Thread newThread(@NonNull Runnable r) {
                    return new Thread(r, "unZipThread");
                }
            });
        threadPoolExecutor.execute(Executors.defaultThreadFactory().newThread(runnable));

        return this;
    }

    private Runnable runnable = new Runnable() {
        @Override
        public void run() {
            copyAssetsToDst(context, srcPath, sdPath);
            if (isSuccess) {
                handler.obtainMessage(SUCCESS).sendToTarget();
            } else {
                handler.obtainMessage(FAILED, errorStr).sendToTarget();
            }
        }
    };

    public void setFileOperateCallback(FileOperateCallback callback) {
        this.callback = callback;
    }

    private void copyAssetsToDst(Context context, String srcPath, String dstPath) {
        try {
            String[] fileNames = context.getAssets().list(srcPath);
            if (fileNames.length > 0) {
                File file = new File(Environment.getExternalStorageDirectory(), dstPath);
                if (!file.exists()) {
                    file.mkdirs();
                }
                for (String fileName : fileNames) {
                    // assets 文件夹下的目录
                    if (!"".equals(srcPath)) {
                        copyAssetsToDst(context, srcPath + File.separator + fileName,
                            dstPath + File.separator + fileName);
                    } else {
                        // assets 文件夹
                        copyAssetsToDst(context, fileName, dstPath + File.separator + fileName);
                    }
                }
            } else {
                File outFile = new File(Environment.getExternalStorageDirectory(), dstPath);
                if(!outFile.exists()){
                    outFile.createNewFile();
                }
                InputStream is = context.getAssets().open(srcPath);
                FileOutputStream fos = new FileOutputStream(outFile);
                byte[] buffer = new byte[1024];
                int byteCount;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
            }
            isSuccess = true;
        } catch (Exception e) {
            e.printStackTrace();
            errorStr = e.getMessage();
            isSuccess = false;
        }
    }

    public void onDestroy() {
        if (threadPoolExecutor != null && runnable != null) {
            threadPoolExecutor.remove(runnable);
        }

        if (handler != null) {
            handler.removeMessages(SUCCESS);
            handler.removeMessages(FAILED);
            handler = null;
        }
        instance = null;
        callback = null;
        context = null;

    }

    public interface FileOperateCallback {
        /**
         * copy success
         */
        void onSuccess();

        /**
         * copy fail
         * @param error 错误信息
         */
        void onFailed(String error);
    }

}
