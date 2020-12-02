package com.cicada.player.utils;

import android.content.Context;
import android.util.Log;

public class Logger {

    private static String TAG = "Logger";

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }

    /**
     * 日志等级
     */
    /****
     * Log level
     */
    public enum LogLevel {

        /**
         * 无
         */
        /****
         * None
         */
        AF_LOG_LEVEL_NONE(0),
        /**
         * 崩溃
         */
        /****
         * Fatal
         */
        AF_LOG_LEVEL_FATAL(8),
        /**
         * 错误
         */
        /****
         * Error
         */
        AF_LOG_LEVEL_ERROR(16),
        /**
         * 警告
         */
        /****
         * Warning
         */
        AF_LOG_LEVEL_WARNING(24),
        /**
         * 信息
         */
        /****
         * Info
         */
        AF_LOG_LEVEL_INFO(32),
        /**
         * 调试
         */
        /****
         * Debug
         */
        AF_LOG_LEVEL_DEBUG(48),
        /**
         * 堆栈
         */
        /****
         * Trace
         */
        AF_LOG_LEVEL_TRACE(56);

        private int mValue;

        private LogLevel(int value) {
            mValue = value;
        }

        public static LogLevel convert(int logLevel) {
            LogLevel level = LogLevel.AF_LOG_LEVEL_NONE;
            for (LogLevel item : LogLevel.values()) {
                if (item.getValue() == logLevel) {
                    level = item;
                    break;
                }
            }
            return level;
        }

        public int getValue() {
            return mValue;
        }

    }

    /**
     * 日志回调接口
     */
    /****
     * Log callback.
     */
    public interface OnLogCallback {
        /**
         *
         * @param level 日志等级。 见{@linkplain LogLevel}
         * @param msg 日志信息
         */
        /****
         *
         * @param level The level of the log. See {@linkplain LogLevel}.
         * @param msg The log data.
         */
        public void onLog(LogLevel level, String msg);
    }

    private static volatile Logger sInstance = null;
    private static Context sAppContext = null;
    private final Object logCallbackLock = new Object();
    private OnLogCallback mLogCallback = null;
    private boolean mEnableConsoleLog = true;

    private Logger() {

    }

    /**
     * 获取Logger的单例
     * @param context 上下文
     * @return Logger的单例
     */
    /****
     * gets the singleton of the Logger
     * @param context
     * @return singleton for Logger
     */
    public static Logger getInstance(Context context) {
        if (sInstance == null) {
            synchronized (Logger.class) {
                if (sInstance == null) {
                    sInstance = new Logger();
                    sInstance.setLogLevel(LogLevel.AF_LOG_LEVEL_INFO);
                    if (context != null) {
                        sAppContext = context.getApplicationContext();
                    }
                }
            }
        }
        return sInstance;
    }

    /**
     * 设置日志回调
     * @param callback 回调。
     */
    /****
     * Set a log callback.
     * @param callback The log callback.
     */
    public void setLogCallback(OnLogCallback callback) {
        synchronized (logCallbackLock) {
            mLogCallback = callback;
        }
    }

    /**
     * 获取当前设置的日志回调
     * @return 日志回调
     */
    /****
     * get the log callback
     * @return log callback
     */
    public OnLogCallback getLogCallback() {
        synchronized (logCallbackLock) {
            return mLogCallback;
        }
    }

    private LogLevel mCurrentLogLevel = LogLevel.AF_LOG_LEVEL_INFO;

    /**
     * 设置日志回调
     * @param logLevel 日志等级。 见{@linkplain LogLevel}。
     */
    /****
     * Set a log callback.
     * @param logLevel The level of the log. See {@linkplain LogLevel}.
     */
    public void setLogLevel(LogLevel logLevel) {
        mCurrentLogLevel = logLevel;
        nSetLogLevel(logLevel.getValue());
    }

    /**
     * 获取日志等级
     * @return 日志等级。见{@linkplain LogLevel}。
     */
    /****
     * get the log level
     * @return log level. See {@linkplain LogLevel}.
     */
    public LogLevel getLogLevel() {
        int logLevel = nGetLogLevel();
        return LogLevel.convert(logLevel);
    }


    /**
     * 是否开启控制台日志打印
     * @param bEnabled
     */
    /****
     * whether to enable console log printing
     * @param bEnabled
     */
    public void enableConsoleLog(boolean bEnabled) {
        mEnableConsoleLog = bEnabled;
        nEnableConsoleLog(bEnabled);
    }

    private static native void nSetLogLevel(int level);

    private static native int nGetLogLevel();

    private static native void nEnableConsoleLog(boolean enable);

    private static void nOnLogCallback(int level, byte[] msg) {
        LogLevel logLevel = getLevel(level);
        String msgStr = new String(msg).trim();

        if (sAppContext != null) {
            Logger logger = getInstance(sAppContext);
            logger.callback(logLevel, msgStr);
        }
    }

    private void callback(LogLevel logLevel, String msgStr) {
        synchronized (logCallbackLock) {
            if (mLogCallback != null) {
                mLogCallback.onLog(logLevel, msgStr);
            }
        }
    }

    private static LogLevel getLevel(int level) {
        switch (level) {
            case 0:
                return LogLevel.AF_LOG_LEVEL_NONE;
            case 8:
                return LogLevel.AF_LOG_LEVEL_FATAL;
            case 16:
                return LogLevel.AF_LOG_LEVEL_ERROR;
            case 24:
                return LogLevel.AF_LOG_LEVEL_WARNING;
            case 32:
                return LogLevel.AF_LOG_LEVEL_INFO;
            case 48:
                return LogLevel.AF_LOG_LEVEL_DEBUG;
            case 56:
                return LogLevel.AF_LOG_LEVEL_TRACE;
            default:
                return LogLevel.AF_LOG_LEVEL_DEBUG;
        }
    }

    public static void d(String tag, String msg) {
        log(LogLevel.AF_LOG_LEVEL_DEBUG, tag, msg);
    }

    public static void i(String tag, String msg) {
        log(LogLevel.AF_LOG_LEVEL_INFO, tag, msg);
    }

    public static void v(String tag, String msg) {
        log(LogLevel.AF_LOG_LEVEL_TRACE, tag, msg);
    }

    public static void w(String tag, String msg) {
        log(LogLevel.AF_LOG_LEVEL_WARNING, tag, msg);
    }

    public static void e(String tag, String msg) {
        log(LogLevel.AF_LOG_LEVEL_ERROR, tag, msg);
    }

    private static void log(LogLevel logLevel, String tag, String msg) {
        LogLevel setLogLevel = Logger.getInstance(sAppContext).mCurrentLogLevel;
        boolean enableConsole = Logger.getInstance(sAppContext).mEnableConsoleLog;
        if (setLogLevel.getValue() < logLevel.getValue() || !enableConsole) {
            return;
        }

        if (logLevel == LogLevel.AF_LOG_LEVEL_TRACE) {
            Log.v(tag, msg);
        } else if (logLevel == LogLevel.AF_LOG_LEVEL_DEBUG) {
            Log.d(tag, msg);
        } else if (logLevel == LogLevel.AF_LOG_LEVEL_INFO) {
            Log.i(tag, msg);
        } else if (logLevel == LogLevel.AF_LOG_LEVEL_WARNING) {
            Log.w(tag, msg);
        } else if (logLevel == LogLevel.AF_LOG_LEVEL_ERROR) {
            Log.e(tag, msg);
        }
    }
}
