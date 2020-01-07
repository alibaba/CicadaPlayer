package com.cicada.player.demo.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Environment;
import android.text.TextUtils;

import com.cicada.player.demo.bean.PlayerMediaInfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;

/**
 * 文件工具类
 */
public class FileUtils {

    /**
     * 判断文件是否存在
     */
    public static boolean isFileExist(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return false;
        }

        File file = new File(filePath);
        return (file.exists() && file.isFile());
    }

    /**
     * 拷贝文件
     *
     */
    public static boolean copyFromAssetToSdcard(Context context, String assetFilename, String dstPath)
            throws IOException {
        InputStream source = null;
        OutputStream destination = null;
        try {
            source = context.getAssets().open(new File(assetFilename).getPath());
            File destinationFile = new File(dstPath, assetFilename);

            destinationFile.getParentFile().mkdirs();
            destination = new FileOutputStream(destinationFile);
            byte[] buffer = new byte[1024];
            int nread;

            while ((nread = source.read(buffer)) != -1) {
                if (nread == 0) {
                    nread = source.read();
                    if (nread < 0) {
                        break;
                    }
                    destination.write(nread);
                    continue;
                }
                destination.write(buffer, 0, nread);
            }
        } finally {
            if (source != null) {
                try {
                    source.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            if (destination != null) {
                try {
                    destination.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
        return true;
    }

    public static String readUrl(String filePath, String charsetName) {
        File file = new File(filePath);
        StringBuilder sb = new StringBuilder();
        if (!file.isFile()) {
            return null;
        }
        BufferedReader reader = null;
        try {
            InputStreamReader is = new InputStreamReader(new FileInputStream(file), charsetName);
            reader = new BufferedReader(is);
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
            reader.close();
            return sb.toString();
        } catch (IOException e) {
            throw new RuntimeException("IOException occurred. ", e);
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    throw new RuntimeException("IOException occurred. ", e);
                }
            }
        }
    }

    /**
     * 从指定文件夹中获取.mp4 .flv .m3u8文件
     * 注意:只解析sdcard根目录,如果是子目录中的文件,则获取不到
     */
    public static ArrayList<PlayerMediaInfo.TypeInfo> getPathList(File groupPath) {
        ArrayList<PlayerMediaInfo.TypeInfo> typeInfos = new ArrayList<>();
        File[] files = groupPath.listFiles();
        if(files == null){
            return null;
        }
        for (int i = 0; i < files.length; i++) {
            File childFile = groupPath.listFiles()[i];
            //假如是目录的话就继续调用getSDcardFile()将childFile作为参数传递的方法里面
            if (childFile.isDirectory()) {
                continue;

            } else {
                //如果是文件的话,判断是不是以.mp3结尾,是就加入到List里面
                if (childFile.toString().endsWith(".mp4")
                        || childFile.toString().endsWith(".MP4")
                        || childFile.toString().endsWith(".mp3")
                        || childFile.toString().endsWith(".MP3")
                        || childFile.toString().endsWith(".flv")
                        || childFile.toString().endsWith(".FLV")
                        || childFile.toString().endsWith(".m3u8")
                        || childFile.toString().endsWith(".M3U8")
                        || childFile.toString().endsWith(".mov")
                        || childFile.toString().endsWith(".MOV")) {
                    PlayerMediaInfo.TypeInfo typeInfo = new PlayerMediaInfo.TypeInfo();
                    typeInfo.setUrl(childFile.getAbsolutePath());
                    typeInfo.setName(childFile.getName());
                    typeInfo.setType("URL");

                    typeInfos.add(typeInfo);
                }
            }
        }
        return typeInfos;
    }

    /**
     * 保存图片到本地
     */
    public static String saveBitmap(Bitmap bitmap){
        String environmentPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File f = new File(environmentPath, "snapShot_"+System.currentTimeMillis()+".png");
        FileOutputStream out = null;
        if (f.exists()) {
            f.delete();
        }
        try {
            out = new FileOutputStream(f);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
            out.flush();
            out.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally{
            if(out != null){
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return f.getAbsolutePath();
    }

    /**
     * 格式化大小
     */
    public static String formatSize(long size) {
        int kb = (int) (size / 1024f);
        if (kb < 1024) {
            return kb + "KB";
        }

        int mb = (int) (kb / 1024f);
        return mb + "MB";
    }


    /**
     * 先根遍历序递归删除文件夹
     *https://blog.csdn.net/bruce128/article/details/79072260
     * @param dirFile 要被删除的文件或者目录
     * @return 删除成功返回true, 否则返回false
     */
    public static boolean deleteFile(File dirFile) {
        // 如果dir对应的文件不存在，则退出
        if (!dirFile.exists()) {
            return false;
        }

        if (dirFile.isFile()) {
            return dirFile.delete();
        } else {

            for (File file : dirFile.listFiles()) {
                deleteFile(file);
            }
        }

        return dirFile.delete();
    }
}
