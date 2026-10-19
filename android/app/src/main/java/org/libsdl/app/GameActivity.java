package org.libsdl.app;

import android.content.SharedPreferences;
import android.os.Bundle;
import java.io.File;

/** SDL game Activity for the statically linked SDL3 build. */
public final class GameActivity extends SDLActivity {
    private static final String PREFS = "pes2013_launcher";
    private static final String GAME_PATH = "game_path";

    @Override
    protected void onCreate(Bundle state) {
        SessionLogger.start(this, "GameActivity");
        String path = getSharedPreferences(PREFS, MODE_PRIVATE).getString(GAME_PATH, "");
        SessionLogger.log(this, "GAME_ACTIVITY_CREATED gameRoot=" + path);
        File xex = new File(path, "default.xex");
        SessionLogger.log(this, "XEX_CHECK path=" + xex.getAbsolutePath() +
                " exists=" + xex.isFile() + " bytes=" + (xex.isFile() ? xex.length() : 0));
        super.onCreate(state);
    }

    @Override
    protected String[] getLibraries() {
        // SDL3 is linked into libmain.so for this arm64 build; do not ask SDL
        // to dlopen a separate libSDL3.so that is not packaged in the APK.
        return new String[] { "main" };
    }

    @Override
    protected String[] getArguments() {
        SharedPreferences prefs = getSharedPreferences(PREFS, MODE_PRIVATE);
        String path = prefs.getString(GAME_PATH, null);
        if (path == null || !new File(path).isDirectory()) {
            SessionLogger.log(this, "GAME_ARGUMENTS empty reason=game_root_missing");
            return new String[0];
        }
        String logPath = SessionLogger.currentPath();
        if (logPath == null) {
            SessionLogger.log(this, "GAME_ARGUMENTS game_data_root=" + path +
                    " native_log=unavailable");
            return new String[] { "--game_data_root=" + path };
        }
        SessionLogger.log(this, "GAME_ARGUMENTS game_data_root=" + path +
                " native_log=" + logPath + " vulkan_loader=system_libvulkan.so");
        return new String[] {
                "--game_data_root=" + path,
                "--log_file=" + logPath
        };
    }

    @Override
    protected void main() {
        SessionLogger.log(this, "SDL_JNI_ENTER");
        try {
            super.main();
            SessionLogger.log(this, "SDL_JNI_RETURN");
        } catch (Throwable error) {
            SessionLogger.log(this, "SDL_JNI_THROWABLE " + error);
            SessionLogger.append(error);
        }
    }

    @Override
    protected void onDestroy() {
        SessionLogger.log(this, "GAME_ACTIVITY_DESTROYED");
        SessionLogger.close(this, "GameActivity");
        super.onDestroy();
    }
}
