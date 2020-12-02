package com.cicada.player.demo.util;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;

public class HttpClientHelper {
    private static final int CONNECTION_TIMEOUT = 10000;

    public static byte[] post(String requestUrl, byte[] body) {
        URLConnection urlConnection = null;
        InputStream in = null;
        ByteArrayOutputStream outputStream = null;
        try {
            URL url = new URL(requestUrl);
            urlConnection = url.openConnection();
            HttpURLConnection connection = (HttpURLConnection) urlConnection;
            connection.setRequestMethod("POST");
            connection.setDoOutput(body != null);
            connection.setDoInput(true);
            connection.setConnectTimeout(CONNECTION_TIMEOUT);
            connection.setReadTimeout(CONNECTION_TIMEOUT);
            if (body != null) {
                connection.setFixedLengthStreamingMode(body.length);
                connection.connect();
                OutputStream os = connection.getOutputStream();
                os.write(body);
                os.close();
            } else {
                connection.connect();
            }

            if (connection.getResponseCode() == HttpURLConnection.HTTP_OK) {
                in = connection.getInputStream();
                byte[] buffer = new byte[1024 * 4];
                outputStream = new ByteArrayOutputStream();
                int bytesRead;
                while ((bytesRead = in.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, bytesRead);
                }
                byte[] bytes = outputStream.toByteArray();
                return bytes;
            }

        } catch (Exception e) {
//            e.printStackTrace();
        } finally {
            try {
                if (in != null) {
                    in.close();
                }

                if (outputStream != null) {
                    outputStream.close();
                }

            } catch (IOException e) {

            }
            if (urlConnection != null) {
                ((HttpURLConnection) urlConnection).disconnect();
            }
        }
        return null;
    }

}
