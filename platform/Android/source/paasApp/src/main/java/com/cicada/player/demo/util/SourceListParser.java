package com.cicada.player.demo.util;

import android.content.Context;
import android.os.Environment;

import com.cicada.player.demo.bean.PlayerMediaInfo;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 解析播放资源
 */
public class SourceListParser {

    public static final String URL_LINKS = "URL links";
    public static final String HARDWARE_LINKS = "HARDWARE links";

    /**
     * 从sourceList.txt文件中解析数据
     */
    public static List<PlayerMediaInfo> parse(Context context){
        String sourceList;
        try{
            String path = Environment.getExternalStorageDirectory().getAbsolutePath();
            String dstPath = path + "/sourceList.json";
            File file = new File(dstPath);
            if(file.exists()){
                file.delete();
            }
            FileUtils.copyFromAssetToSdcard(context, "sourceList.json", path);
            sourceList = FileUtils.readUrl(dstPath, "UTF-8");
            Gson gson = new Gson();
            List<PlayerMediaInfo> list;
            Type listType = new TypeToken<Collection<PlayerMediaInfo>>() {}.getType();
            list = gson.fromJson(sourceList, listType);
            return list;
        }catch (IOException e){
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 将数据解析成Map集合
     */
    public static Map<String,ArrayList<PlayerMediaInfo.TypeInfo>> handleDate(List<PlayerMediaInfo> list){
        Map<String,ArrayList<PlayerMediaInfo.TypeInfo>> mapDate = new HashMap<>();
        ArrayList<PlayerMediaInfo.TypeInfo> urlTypeInfo = null;
        for (PlayerMediaInfo mediaInfo : list) {
            urlTypeInfo = new ArrayList<>();
            String name = mediaInfo.getName();
            List<PlayerMediaInfo.TypeInfo> samples = mediaInfo.getSamples();
            for (PlayerMediaInfo.TypeInfo sample : samples) {
                if(URL_LINKS.equals(name)){
                    urlTypeInfo.add(sample);
                }else if(HARDWARE_LINKS.equals(name)){
                    urlTypeInfo.add(sample);
                }else{
                    urlTypeInfo.add(sample);
                }
            }
            mapDate.put(name,urlTypeInfo);
        }
        return mapDate;
    }

    /**
     * 将数据存储到List集合中
     */
    public static List<String> getDateTitleKey(List<PlayerMediaInfo> list){
        List<String> typeNameList = new ArrayList<>();
        for (PlayerMediaInfo mediaInfo : list) {
            String typeName = mediaInfo.getName();
            if(!typeNameList.contains(typeName)){
                typeNameList.add(typeName);
            }
        }
        return typeNameList;
    }
}
