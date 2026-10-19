package org.libsdl.app;

import android.content.Context;
import android.content.SharedPreferences;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/** Small file logger for launcher/game lifecycle diagnostics. */
public final class SessionLogger {
    private static final String PREFS = "pes2013_launcher";
    private static final String ENABLED = "auto_log";
    private static final Object LOCK = new Object();
    private static File currentFile;
    private static Thread.UncaughtExceptionHandler previousHandler;

    private SessionLogger() {}

    public static boolean isEnabled(Context context) {
        return context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
                .getBoolean(ENABLED, true);
    }

    public static void setEnabled(Context context, boolean enabled) {
        context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
                .edit().putBoolean(ENABLED, enabled).apply();
        if (!enabled) log(context, "AUTO_LOG_DISABLED");
    }

    public static File start(Context context, String component) {
        if (!isEnabled(context)) return null;
        synchronized (LOCK) {
            try {
                File base = new File(context.getExternalFilesDir(null), "logs");
                if (!base.exists() && !base.mkdirs()) return null;
                String stamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
                currentFile = new File(base, "pes2013_" + stamp + ".log");
                appendLocked("SESSION_START component=" + component);
                appendLocked("package=" + context.getPackageName());
                appendLocked("android=" + android.os.Build.VERSION.RELEASE + " sdk=" + android.os.Build.VERSION.SDK_INT);
                appendLocked("abi=arm64-v8a");
                if (previousHandler == null) {
                    previousHandler = Thread.getDefaultUncaughtExceptionHandler();
                    Thread.setDefaultUncaughtExceptionHandler((thread, error) -> {
                        append("UNCAUGHT_EXCEPTION thread=" + thread.getName());
                        append(error);
                        if (previousHandler != null) previousHandler.uncaughtException(thread, error);
                    });
                }
                return currentFile;
            } catch (Exception ignored) {
                return null;
            }
        }
    }

    public static void log(Context context, String message) {
        if (isEnabled(context)) append(message);
    }

    public static void append(String message) {
        synchronized (LOCK) {
            appendLocked(message);
        }
    }

    public static void append(Throwable error) {
        synchronized (LOCK) {
            if (error != null) {
                appendLocked(error.toString());
                for (StackTraceElement line : error.getStackTrace()) appendLocked("  at " + line);
            }
        }
    }

    public static void close(Context context, String component) {
        if (!isEnabled(context)) return;
        append("SESSION_END component=" + component);
    }

    public static File latestFile(Context context) {
        File base = new File(context.getExternalFilesDir(null), "logs");
        File[] files = base.listFiles((dir, name) -> name.endsWith(".log"));
        if (files == null || files.length == 0) return currentFile;
        File latest = files[0];
        for (File file : files) if (file.lastModified() > latest.lastModified()) latest = file;
        return latest;
    }

    public static String currentPath() {
        synchronized (LOCK) {
            return currentFile == null ? null : currentFile.getAbsolutePath();
        }
    }

    private static void appendLocked(String message) {
        if (currentFile == null) return;
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(currentFile, true))) {
            String time = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.US).format(new Date());
            writer.write(time + " " + message);
            writer.newLine();
        } catch (IOException ignored) { }
    }
}
