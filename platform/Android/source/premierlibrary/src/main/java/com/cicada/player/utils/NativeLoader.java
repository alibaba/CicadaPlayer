package com.cicada.player.utils;

public class NativeLoader {

    private static boolean playerLoaded = false;

    public synchronized static void loadPlayer() {
        if (playerLoaded) {
            return;
        }

        try {
            System.loadLibrary("alivcffmpeg");
            System.loadLibrary("CicadaPlayer");
            playerLoaded = true;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
