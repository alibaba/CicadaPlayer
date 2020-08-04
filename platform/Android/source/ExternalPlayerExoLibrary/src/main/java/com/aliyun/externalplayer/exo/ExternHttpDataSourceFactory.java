package com.aliyun.externalplayer.exo;

import android.support.annotation.Nullable;
import android.text.TextUtils;

import com.google.android.exoplayer2.upstream.DefaultHttpDataSource;
import com.google.android.exoplayer2.upstream.HttpDataSource;
import com.google.android.exoplayer2.upstream.TransferListener;

import java.util.ArrayList;
import java.util.List;

public class ExternHttpDataSourceFactory extends HttpDataSource.BaseFactory {

    private final @Nullable
    TransferListener listener;
    private final boolean allowCrossProtocolRedirects;
    List<String> mHttpHeaders = new ArrayList<>();
    private String userAgent;
    private int connectTimeoutMillis;
    private int readTimeoutMillis;

    public ExternHttpDataSourceFactory() {
        this(ExternHttpDataSourceFactory.class.getSimpleName(), null);
    }

    /**
     * Constructs a DefaultHttpDataSourceFactory. Sets {@link
     * DefaultHttpDataSource#DEFAULT_CONNECT_TIMEOUT_MILLIS} as the connection timeout, {@link
     * DefaultHttpDataSource#DEFAULT_READ_TIMEOUT_MILLIS} as the read timeout and disables
     * cross-protocol redirects.
     *
     * @param userAgent The User-Agent string that should be used.
     */
    public ExternHttpDataSourceFactory(String userAgent) {
        this(userAgent, null);
    }

    /**
     * Constructs a DefaultHttpDataSourceFactory. Sets {@link
     * DefaultHttpDataSource#DEFAULT_CONNECT_TIMEOUT_MILLIS} as the connection timeout, {@link
     * DefaultHttpDataSource#DEFAULT_READ_TIMEOUT_MILLIS} as the read timeout and disables
     * cross-protocol redirects.
     *
     * @param userAgent The User-Agent string that should be used.
     * @param listener  An optional listener.
     * @see #ExternHttpDataSourceFactory(String, TransferListener, int, int, boolean)
     */
    public ExternHttpDataSourceFactory(String userAgent, @Nullable TransferListener listener) {
        this(userAgent, listener, DefaultHttpDataSource.DEFAULT_CONNECT_TIMEOUT_MILLIS,
                DefaultHttpDataSource.DEFAULT_READ_TIMEOUT_MILLIS, false);
    }

    /**
     * @param userAgent                   The User-Agent string that should be used.
     * @param connectTimeoutMillis        The connection timeout that should be used when requesting remote
     *                                    data, in milliseconds. A timeout of zero is interpreted as an infinite timeout.
     * @param readTimeoutMillis           The read timeout that should be used when requesting remote data, in
     *                                    milliseconds. A timeout of zero is interpreted as an infinite timeout.
     * @param allowCrossProtocolRedirects Whether cross-protocol redirects (i.e. redirects from HTTP
     *                                    to HTTPS and vice versa) are enabled.
     */
    public ExternHttpDataSourceFactory(
            String userAgent,
            int connectTimeoutMillis,
            int readTimeoutMillis,
            boolean allowCrossProtocolRedirects) {
        this(
                userAgent,
                /* listener= */ null,
                connectTimeoutMillis,
                readTimeoutMillis,
                allowCrossProtocolRedirects);
    }

    /**
     * @param userAgent                   The User-Agent string that should be used.
     * @param listener                    An optional listener.
     * @param connectTimeoutMillis        The connection timeout that should be used when requesting remote
     *                                    data, in milliseconds. A timeout of zero is interpreted as an infinite timeout.
     * @param readTimeoutMillis           The read timeout that should be used when requesting remote data, in
     *                                    milliseconds. A timeout of zero is interpreted as an infinite timeout.
     * @param allowCrossProtocolRedirects Whether cross-protocol redirects (i.e. redirects from HTTP
     *                                    to HTTPS and vice versa) are enabled.
     */
    public ExternHttpDataSourceFactory(
            String userAgent,
            @Nullable TransferListener listener,
            int connectTimeoutMillis,
            int readTimeoutMillis,
            boolean allowCrossProtocolRedirects) {
        this.userAgent = userAgent;
        this.listener = listener;
        this.connectTimeoutMillis = connectTimeoutMillis;
        this.readTimeoutMillis = readTimeoutMillis;
        this.allowCrossProtocolRedirects = allowCrossProtocolRedirects;
    }

    public void addHttpHeaders(String httpHeader) {
        if (!TextUtils.isEmpty(httpHeader)) {
            mHttpHeaders.add(httpHeader);
        }
    }

    public void clearHttpHeaders() {
        mHttpHeaders.clear();
    }

    public void setConnectTimeoutMillis(int timeoutMillis) {
        connectTimeoutMillis = timeoutMillis;
    }

    public void setReadTimeoutMillis(int timeoutMillis) {
        readTimeoutMillis = timeoutMillis;
    }

    @Override
    protected DefaultHttpDataSource createDataSourceInternal(
            HttpDataSource.RequestProperties defaultRequestProperties) {
        DefaultHttpDataSource dataSource =
                new DefaultHttpDataSource(
                        TextUtils.isEmpty(userAgent) ? ExternHttpDataSourceFactory.class.getSimpleName() : userAgent,
                        /* contentTypePredicate= */ null,
                        connectTimeoutMillis,
                        readTimeoutMillis,
                        allowCrossProtocolRedirects,
                        defaultRequestProperties);
        for (String header : mHttpHeaders) {
            String[] keyValue = header.split(":");
            if (keyValue.length != 2) {
                continue;
            }
            dataSource.setRequestProperty(keyValue[0], keyValue[1]);
        }

        if (listener != null) {
            dataSource.addTransferListener(listener);
        }
        return dataSource;
    }

    public void setUserAgent(String userAgent) {
        this.userAgent = userAgent;
    }
}
