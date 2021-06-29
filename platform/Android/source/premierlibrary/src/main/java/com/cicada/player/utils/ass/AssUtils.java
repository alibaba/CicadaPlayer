package com.cicada.player.utils.ass;

public class AssUtils {

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }

    public static AssHeader parseAssHeader(String header) {
        return (AssHeader) nParseAssHeader(header);
    }

    public static AssDialogue parseAssDialogue(AssHeader header, String data) {
        return (AssDialogue) nParseAssDialogue(header, data);
    }

    private static native Object nParseAssHeader(String header);

    private static native Object nParseAssDialogue(Object header, String data);

}
