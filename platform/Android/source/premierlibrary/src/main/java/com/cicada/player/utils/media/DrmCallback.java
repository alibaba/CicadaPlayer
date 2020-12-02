package com.cicada.player.utils.media;

public interface DrmCallback {

    byte[] requestProvision(String defaultUrl, byte[] data);

    byte[] requestKey(String defaultUrl, byte[] data);

}
