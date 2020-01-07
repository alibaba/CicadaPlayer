package com.cicada.player.demo.util;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class VcPlayerLog {

    public static final int LogMode_Off = 0; //关闭
    public static final int LogMode_Console = 1; //控制台
    public static final int LogMode_File = 2; //文件

    public static void enableLog(boolean enable) {
        sLogMode = (enable ? LogMode_Console : LogMode_Off);
    }


    public static class LogLevel {
        public static final LogLevel Verbose = new LogLevel(0, "V");
        public static final LogLevel Debug = new LogLevel(1, "D");
        public static final LogLevel Info = new LogLevel(2, "I");
        public static final LogLevel Warn = new LogLevel(3, "W");
        public static final LogLevel Error = new LogLevel(4, "E");

        private int value = -1;
        private String name = "";

        private LogLevel(int v, String aname) {
            value = v;
            name = aname;
        }

        public int value() {
            return value;
        }

        public String name() {
            return name;
        }

    }

    private static LogLevel sLogLevel = LogLevel.Verbose;
    private static int sLogMode = LogMode_Console;
    private static BufferedWriter sLogWriter = null;
    private static String sAppPackageName = null;


    public static void setLogInfo(Context context, int logMode, LogLevel level, String absPath) {
        if (context != null) {
            sAppPackageName = context.getPackageName();
        }

        if (logMode < 0) {
            sLogMode = LogMode_Off;
        } else {
            sLogMode = logMode;
        }

        if (level == null) {
            sLogLevel = LogLevel.Verbose;
        } else {
            sLogLevel = level;
        }

        closeWriter();
        if (isModeContains(sLogMode, LogMode_File)) {
            createLogWriter(absPath);
        }

        for (OnConfigChangedListener l : listenerList) {
            l.onConfigChanged(level.value(), (logMode & LogMode_Console) == LogMode_Console,
                    (logMode & LogMode_File) == LogMode_File);
        }
    }

    public interface OnConfigChangedListener {
        void onConfigChanged(int level, boolean enableConsole, boolean enableFile);
    }

    private static List<OnConfigChangedListener> listenerList = new ArrayList<OnConfigChangedListener>();

    public static void addOnConfigChangedListener(OnConfigChangedListener l) {
        if (l != null) {
            listenerList.add(l);
            l.onConfigChanged(sLogLevel.value(), (sLogMode & LogMode_Console) == LogMode_Console,
                    (sLogMode & LogMode_File) == LogMode_File);
        }
    }

    public static void removeOnConfigChangedListener(OnConfigChangedListener l) {
        listenerList.remove(l);
    }


    public static void v(String tag, String msg) {
        printLog(tag, msg, LogLevel.Verbose);
    }

    public static void d(String tag, String msg) {
        printLog(tag, msg, LogLevel.Debug);
    }

    public static void i(String tag, String msg) {
        printLog(tag, msg, LogLevel.Info);
    }

    public static void w(String tag, String msg) {
        printLog(tag, msg, LogLevel.Warn);
    }

    public static void e(String tag, String msg) {
        printLog(tag, msg, LogLevel.Error);
    }

    private static void printLog(String tag, String msg, LogLevel targetLevel) {
        consoleLog(tag, msg, targetLevel);
        fileLog(tag, msg, targetLevel);
    }

    private static void fileLog(String tag, String msg, LogLevel targetLevel) {
        if (!isSourceLarger(sLogLevel, targetLevel)) {
            return;
        }

        if (isModeContains(sLogMode, LogMode_File)) {
            String log = formatLog(targetLevel.name(), tag, msg);
            WriteLog(log);
        }
    }

    private static void consoleLog(String tag, String msg, LogLevel targetLevel) {
        if (isSourceLarger(sLogLevel, targetLevel)) {
            return;
        }

        if (isModeContains(sLogMode, LogMode_Console)) {
            if (targetLevel == LogLevel.Verbose) {
                Log.v(tag, msg);
            } else if (targetLevel == LogLevel.Debug) {
                Log.d(tag, msg);
            } else if (targetLevel == LogLevel.Info) {
                Log.i(tag, msg);
            } else if (targetLevel == LogLevel.Warn) {
                Log.w(tag, msg);
            } else if (targetLevel == LogLevel.Error) {
                Log.e(tag, msg);
            } else {

            }
        }
        return;
    }


    private static String formatLog(String levelName, String tag, String msg) {
        StringBuilder stringBuilder = new StringBuilder();

//        07-05 17:32:48.723 16329-16329/com.aliyun.asparaplayer D/StsPlayerActivity: 缓冲结束
        stringBuilder.append(new SimpleDateFormat("MM-dd HH:mm:ss:SSS").format(new Date()));
        stringBuilder.append(" ");
        stringBuilder.append(android.os.Process.myPid()).append("-").append(android.os.Process.myTid());
        stringBuilder.append(" ");
        stringBuilder.append(levelName).append(" ");
        stringBuilder.append(tag).append(": ").append(msg);

        return stringBuilder.toString();
    }

    public static synchronized void WriteLog(String log) {
        if (sLogWriter != null && !TextUtils.isEmpty(log)) {
            try {
                sLogWriter.write(log.trim() + "\n");
                sLogWriter.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static boolean isModeContains(int source, int target) {
        return (source & target) == target;
    }

    private static boolean isSourceLarger(LogLevel source, LogLevel target) {
        return source.value() >= target.value();
    }

    private static void createLogWriter(String absPath) {
        if (TextUtils.isEmpty(absPath)) {
            return;
        }

        boolean ret = false;
        File file = new File(absPath);
        if (!file.exists() || file.isDirectory()) {
            File parent = file.getParentFile();
            if (!parent.exists() || !parent.isDirectory()) {
                ret = parent.mkdirs();
            } else {
                ret = true;
            }

            if (ret) {
                try {
                    ret = file.createNewFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        } else {
            ret = true;
        }

        if (ret) {
            try {
                sLogWriter = new BufferedWriter(new FileWriter(file, true));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 关闭输出文件
     */
    public static void close() {
        closeWriter();

        listenerList.clear();
    }

    private static void closeWriter() {
        if (sLogWriter != null) {
            try {
                sLogWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                sLogWriter = null;
            }
        }
    }

}
