package com.aliyun.externalplayer.exo;

import com.cicada.player.CicadaExternalPlayer;
import com.google.android.exoplayer2.drm.ExoMediaDrm;
import com.google.android.exoplayer2.drm.MediaDrmCallback;

import java.util.UUID;

public class WideVineDrmCallback implements MediaDrmCallback {

    private final CicadaExternalPlayer.OnDRMCallback mDrmCallback;

    public WideVineDrmCallback(CicadaExternalPlayer.OnDRMCallback drmCallback) {
        mDrmCallback = drmCallback;
    }

    @Override
    public byte[] executeProvisionRequest(UUID uuid, ExoMediaDrm.ProvisionRequest request) throws Exception {
        if (mDrmCallback != null) {
            return mDrmCallback.onRequestProvision(request.getDefaultUrl(), request.getData());
        }
        return null;
    }

    @Override
    public byte[] executeKeyRequest(UUID uuid, ExoMediaDrm.KeyRequest request) throws Exception {
        if (mDrmCallback != null) {
            return mDrmCallback.onRequestKey(request.getLicenseServerUrl(), request.getData());
        }
        return null;
    }
}
