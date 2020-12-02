package com.cicada.player.utils.media;

import com.cicada.player.utils.NativeUsed;

import java.util.List;

@NativeUsed
public class EncryptionInfo {
    public String scheme;

    public int crypt_byte_block = 0;
    public int skip_byte_block = 0;

    public byte[] key_id = null;
    public byte[] iv = null;
    public List<SubsampleEncryptionInfo> subsamples = null;

    public void setKeyId(byte[] keyId) {
        this.key_id = keyId;
    }

    public void setIv(byte[] iv) {
        this.iv = iv;
    }

    public void setSubsamples(Object subsamples) {
        this.subsamples = (List<SubsampleEncryptionInfo>) subsamples;
    }

    public void setScheme(String scheme){
        this.scheme = scheme;
    }
}
