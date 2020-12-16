package com.cicada.player.utils;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

@NativeUsed
public class ContentDataSource {

    private static final int SEEK_SET = 0;    /* set file offset to offset */
    private static final int SEEK_CUR = 1;    /* set file offset to current plus offset */
    private static final int SEEK_END = 2;    /* set file offset to EOF plus offset */
    private static final int SEEK_SIZE = 0x10000;

    private static final int ENOENT = 2;
    private static final int EIO = 5;
    private static final int EINVAL = 22;

    private String mUri = null;
    private InputStream mStream = null;
    private int mStreamSize = -1;
    private long mOffset = 0;
    private static Context sContext = null;

    public ContentDataSource() {

    }

    public static void setContext(Context context) {
        if (context != null && sContext == null) {
            sContext = context.getApplicationContext();
        }
    }

    public void setUri(String uri) {
        mUri = uri;
    }

    public int open(int flags) {

        if (TextUtils.isEmpty(mUri)) {
            return -EINVAL;
        }

        if (sContext == null) {
            return -EINVAL;
        }

        ContentResolver contentResolver = sContext.getContentResolver();
        Uri uri = Uri.parse(mUri);

        try {
            mStream = contentResolver.openInputStream(uri);
        } catch (FileNotFoundException e) {
            return -ENOENT;
        }

        if (mStream == null) {
            return -EINVAL;
        }

        try {
            mStreamSize = mStream.available();
        } catch (IOException e) {
            return -EIO;
        }
        return 0;
    }

    public int read(byte[] buffer) {
        if (mStream == null) {
            return -EINVAL;
        }
        int read = -1;
        try {
            read = mStream.read(buffer);
            mOffset += read;
        } catch (IOException e) {
            return -EIO;
        }
        return read;
    }

    public void close() {
        if (mStream != null) {
            try {
                mStream.close();
            } catch (IOException e) {
            }
        }
    }

    public long seek(long offset, int whence) {
        if (mStream == null) {
            return -EINVAL;
        }

        if (whence == SEEK_SIZE) {
            if (mStreamSize <= 0) {
                return -EINVAL;
            } else {
                return mStreamSize;
            }
        } else {
            long targetOffset = 0;
            if (whence == SEEK_END) {
                try {
                    targetOffset = mStream.available();
                } catch (IOException e) {
                    return -EIO;
                }
            } else if (whence == SEEK_SET) {
                targetOffset = offset - mOffset;
            } else if (whence == SEEK_CUR) {
                targetOffset = offset;
            } else {
                return -EINVAL;
            }

            try {
                long skipBytes = mStream.skip(targetOffset);
                mOffset += skipBytes;
                return mOffset;
            } catch (IOException e) {
                return -EIO;
            }

        }

    }
}
