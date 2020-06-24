package com.cicada.player;


public class CicadaPlayerGlobalSettings {

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }

    /**
     * 设置域名对应的解析ip
     * @param host 域名，需指定端口（http默认端口80，https默认端口443）。例如player.alicdn.com:80
     * @param ip 相应的ip，设置为空字符串清空设定。
     */
    /****
     * Set a DNS ip to resolve the host.
     * @param host The host. Need to specify the port(http defualt port is 80，https default port is 443). E.g player.alicdn.com:80
     * @param ip The ip address, set as empty string to clear the setting.
     */
    public static void setDNSResolve(String host, String ip){
        nSetDNSResolve(host, ip);
    }

    /**
     * 设置网络ip解析类型
     * @param type 见 {@link CicadaPlayer.IPResolveType}
     */
    public void setIPResolveType(CicadaPlayer.IPResolveType type) {
        nSetIPResolveType(type.ordinal());
    }

    private static native void nSetDNSResolve(String host, String ip);

    private static native void nSetIPResolveType(int type);

}
