package com.cicada.player.utils.media;

import android.annotation.SuppressLint;
import android.media.MediaDrm;
import android.media.NotProvisionedException;
import android.media.UnsupportedSchemeException;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Base64;

import com.cicada.player.utils.Logger;
import com.cicada.player.utils.NativeUsed;

import java.util.UUID;

import static android.media.MediaDrm.EVENT_KEY_REQUIRED;
import static android.media.MediaDrm.EVENT_PROVISION_REQUIRED;

@NativeUsed
public class DrmSessionManager {

    private static final String TAG = DrmSessionManager.class.getSimpleName();

    private static final String WIDEVINE_FORMAT = "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
    public static final UUID WIDEVINE_UUID = new UUID(0xEDEF8BA979D64ACEL, 0xA3C827DCD51D21EDL);

    private long mNativeInstance = 0;

    public DrmSessionManager(long nativeInstance) {
        mNativeInstance = nativeInstance;
    }

    private DrmSession drmSession = null;

    public static int SESSION_STATE_ERROR = -1;
    public static int SESSION_STATE_IDLE = -2;
    public static int SESSION_STATE_OPENED = 0;

    public static int ERROR_CODE_NONE = 0;
    public static int ERROR_CODE_UNSUPPORT_SCHEME = 1;
    public static int ERROR_CODE_RESOURCE_BUSY = 2;
    public static int ERROR_CODE_KEY_RESPONSE_NULL = 3;
    public static int ERROR_CODE_PROVISION_RESPONSE_NULL = 4;
    public static int ERROR_CODE_DENIED_BY_SERVER = 5;
    public static int ERROR_CODE_RELEASED = 6;
    public static int ERROR_CODE_PROVISION_FAIL = 7;

    private static class DrmInfo {
        public String licenseUrl = null;
        public String keyUrl = null;
        public String keyFormat = null;
        public String mime = null;

        public boolean isSame(DrmInfo info) {
            if (info == null) {
                return false;
            }

            if (!areEqual(keyUrl, info.keyUrl)) {
                return false;
            }

            if (!areEqual(licenseUrl, info.licenseUrl)) {
                return false;
            }

            if (!areEqual(keyFormat, info.keyFormat)) {
                return false;
            }

            return true;
        }

        private static boolean areEqual(Object o1, Object o2) {
            return o1 == null ? o2 == null : o1.equals(o2);
        }
    }

    private class DrmSession {
        public DrmInfo drmInfo = null;
        public MediaDrm mediaDrm = null;
        public byte[] sessionId = null;

        public int state = SESSION_STATE_IDLE;

        private HandlerThread requestHandlerThread = null;
        public Handler requestHandler = null;

        public DrmSession(DrmInfo info) {
            drmInfo = info;

            requestHandlerThread = new HandlerThread("DrmRequestHanderThread");
            requestHandlerThread.start();
            requestHandler = new Handler(requestHandlerThread.getLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    int event = msg.what;
//                    Logger.v(TAG, "  handleMessage event = " + event);
                    if (event == EVENT_PROVISION_REQUIRED) {
                        requestProvision();
                    } else if (event == EVENT_KEY_REQUIRED
                            || event == MediaDrm.EVENT_KEY_EXPIRED) {
                        try {
                            requestKey();
                        } catch (NotProvisionedException e) {
                            requestProvision();
                        }
                    }

                    super.handleMessage(msg);
                }
            };
        }

        public boolean prepare(boolean allowProvisioning) {
            if (mediaDrm == null) {
                try {
                    if (WIDEVINE_FORMAT.equals(drmInfo.keyFormat)) {
                        mediaDrm = new MediaDrm(WIDEVINE_UUID);
                    } else {
                        Logger.e(TAG, " prepare fail : not support format :" + drmInfo.keyFormat);
                        changeState(SESSION_STATE_ERROR, ERROR_CODE_UNSUPPORT_SCHEME);
                        return false;
                    }
                } catch (UnsupportedSchemeException e) {
                    Logger.e(TAG, " prepare fail : " + e.getMessage());
                    changeState(SESSION_STATE_ERROR, ERROR_CODE_UNSUPPORT_SCHEME);
                    return false;
                }

                mediaDrm.setOnEventListener(new MediaDrm.OnEventListener() {
                    @Override
                    public void onEvent(MediaDrm md, byte[] sessionId, int event, int extra, byte[] data) {
                        Logger.d(TAG, " drm Event = " + event + " , extra = " + extra + " , sessionId =  " + sessionId);
                        sendRequest(event, sessionId);
                    }
                });
            }

            try {
                sessionId = mediaDrm.openSession();
                native_updateSessionId(mNativeInstance, sessionId);
                changeState(SESSION_STATE_IDLE, ERROR_CODE_NONE);
                sendRequest(EVENT_KEY_REQUIRED, sessionId);
            } catch (NotProvisionedException e) {
                Logger.e(TAG, " prepare NotProvisionedException : " + e.getMessage());
                if (allowProvisioning) {
                    sendRequest(EVENT_PROVISION_REQUIRED, null);
                } else {
                    changeState(SESSION_STATE_ERROR, ERROR_CODE_PROVISION_FAIL);
                }
                return false;
            } catch (Exception e) {
                Logger.e(TAG, " prepare fail : " + e.getMessage());
                changeState(SESSION_STATE_ERROR, ERROR_CODE_RESOURCE_BUSY);
                return false;
            }

            return true;
        }

        private void sendRequest(int event, byte[] sessionId) {
            Message msg = requestHandler.obtainMessage(event, sessionId);
            requestHandler.sendMessage(msg);
        }

        public boolean release() {

            changeState(SESSION_STATE_ERROR, ERROR_CODE_RELEASED);

            requestHandlerThread.quit();

            if (mediaDrm != null) {
                if (sessionId != null) {
                    mediaDrm.closeSession(sessionId);
                }
                mediaDrm.release();
                mediaDrm = null;
            }

            return true;
        }

        private void requestKey() throws NotProvisionedException {
            Logger.d(TAG, "requestKey state = " + state);
            if (state == SESSION_STATE_ERROR) {
                return;
            }

            try {
                byte[] initData = Base64.decode(drmInfo.keyUrl.substring(drmInfo.keyUrl.indexOf(',')), Base64.DEFAULT);
                MediaDrm.KeyRequest keyRequest = mediaDrm.getKeyRequest(sessionId, initData,
                        drmInfo.mime, MediaDrm.KEY_TYPE_STREAMING, null);
                byte[] requestData = native_requestKey(mNativeInstance, keyRequest.getDefaultUrl(), keyRequest.getData());

                Logger.v(TAG, "requestKey result = " + new String(requestData));

                if (requestData == null) {
                    Logger.e(TAG, "requestKey fail: data = null , url : " + keyRequest.getDefaultUrl());
                    changeState(SESSION_STATE_ERROR, ERROR_CODE_KEY_RESPONSE_NULL);
                    return;
                }

                mediaDrm.provideKeyResponse(sessionId, requestData);
                changeState(SESSION_STATE_OPENED, ERROR_CODE_NONE);
            } catch (Exception e) {
                Logger.e(TAG, "requestKey fail: " + e.getMessage());
                changeState(SESSION_STATE_ERROR, ERROR_CODE_DENIED_BY_SERVER);
            }
        }

        private boolean hasProvideProvision = false;

        private void requestProvision() {

            Logger.d(TAG, "requestProvision  state = " + state);
            if (hasProvideProvision) {
                return;
            }

            MediaDrm.ProvisionRequest request = mediaDrm.getProvisionRequest();
            byte[] provisionData = native_requestProvision(mNativeInstance, request.getDefaultUrl(), request.getData());
            if (provisionData == null) {
                Logger.e(TAG, "requestProvision fail: data = null , url : " + request.getDefaultUrl());
                changeState(SESSION_STATE_ERROR, ERROR_CODE_PROVISION_RESPONSE_NULL);
                return;
            }

            Logger.d(TAG, "requestProvision : data =  " + new String(provisionData));

            try {
                mediaDrm.provideProvisionResponse(provisionData);
                hasProvideProvision = true;
                if (state == SESSION_STATE_IDLE) {
                    prepare(false);
                }
            } catch (Exception e) {
                Logger.e(TAG, "requestProvision fail: " + e.getMessage());
                changeState(SESSION_STATE_ERROR, ERROR_CODE_PROVISION_FAIL);
            }
        }

        private void changeState(int state, int errorCode) {
            this.state = state;
            Logger.d(TAG, "changeState " + state);
            native_changeState(mNativeInstance, state, errorCode);
        }

        @SuppressLint("WrongConstant")
        public boolean isForceInsecureDecoder() {
            if (mediaDrm != null) {
                return Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP &&
                        "L3".equals(mediaDrm.getPropertyString("securityLevel"));
            } else {
                return false;
            }
        }
    }

    @NativeUsed
    public synchronized void requireSession(String keyUrl, String keyFormat, String mime, String licenseUrl) {
        Logger.d(TAG, "requireSessionInner info = " + keyFormat);

        DrmInfo info = new DrmInfo();
        info.licenseUrl = licenseUrl;
        info.keyFormat = keyFormat;
        info.keyUrl = keyUrl;
        info.mime = mime;

        requireSessionInner(info);
    }

    private void requireSessionInner(DrmInfo info) {
        if (drmSession == null) {
            drmSession = new DrmSession(info);
            drmSession.prepare(true);
        }
    }

    @NativeUsed
    public synchronized void releaseSession() {
        Logger.d(TAG, "releaseSession");
        if (drmSession != null) {
            drmSession.release();
            drmSession = null;
        }
    }

    @NativeUsed
    public boolean isForceInsecureDecoder() {
        if (drmSession != null) {
            return drmSession.isForceInsecureDecoder();
        } else {
            return false;
        }
    }

    @SuppressLint("ObsoleteSdkInt")
    @NativeUsed
    public static boolean supportDrm(String format) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2) {
            return false;
        } else if (WIDEVINE_FORMAT.equals(format) && !MediaDrm.isCryptoSchemeSupported(WIDEVINE_UUID)) {
            return false;
        }

        return true;
    }

    protected native byte[] native_requestProvision(long nativeInstance, String url, byte[] data);

    protected native byte[] native_requestKey(long nativeInstance, String url, byte[] data);

    protected native void native_changeState(long nativeInstance, int state, int errorCode);

    protected native void native_updateSessionId(long nativeInstance, byte[] sessionId);
}
