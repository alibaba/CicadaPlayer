package com.fplayer.flutter_cicadaplayer;

import androidx.annotation.NonNull;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;
import io.flutter.plugin.common.PluginRegistry.Registrar;

/**
 * FlutterCicadaPlayerPlugin
 */
public class FlutterCicadaPlayerPlugin implements FlutterPlugin, MethodCallHandler {
    /// The MethodChannel that will the communication between Flutter and native Android
    ///
    /// This local reference serves to register the plugin with the Flutter Engine and unregister it
    /// when the Flutter Engine is detached from the Activity
    private FlutterPluginBinding flutterPluginBinding;
    private FlutterCicadaPlayerView mFlutterCicadaPlayerView;
    private FlutterCicadaPlayer mFlutterCicadaPlayer;


    @Override public void onAttachedToEngine(@NonNull FlutterPluginBinding flutterPluginBinding)
    {
        this.flutterPluginBinding = flutterPluginBinding;
        mFlutterCicadaPlayerView = new FlutterCicadaPlayerView(flutterPluginBinding);
        flutterPluginBinding.getPlatformViewRegistry().registerViewFactory("flutter_cicadaplayer_render_view", mFlutterCicadaPlayerView);
        MethodChannel mCicadaPlayerFactoryMethodChannel =
                new MethodChannel(flutterPluginBinding.getBinaryMessenger(), "plugins.flutter_cicadaplayer_factory");
        mCicadaPlayerFactoryMethodChannel.setMethodCallHandler(this);
    }

    //   This static function is optional and equivalent to onAttachedToEngine. It supports the old
    //   pre-Flutter-1.12 Android projects. You are encouraged to continue supporting
    //   plugin registration via this function while apps migrate to use the new Android APIs
    //   post-flutter-1.12 via https://flutter.dev/go/android-project-migration.
    //
    //   It is encouraged to share logic between onAttachedToEngine and registerWith to keep
    //   them functionally equivalent. Only one of onAttachedToEngine or registerWith will be called
    //   depending on the user's project. onAttachedToEngine or registerWith must both be defined
    //   in the same class.
    public static void registerWith(Registrar registrar)
    {}

    @Override public void onMethodCall(@NonNull MethodCall call, @NonNull Result result)
    {
        switch (call.method) {
            case "createCicadaPlayer":
                mFlutterCicadaPlayer = new FlutterCicadaPlayer(flutterPluginBinding);
                mFlutterCicadaPlayerView.setPlayer(mFlutterCicadaPlayer.getCicadaPlayer());
                result.success(null);
                break;
            default:
                break;
        }
    }

    @Override public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding)
    {}
}
