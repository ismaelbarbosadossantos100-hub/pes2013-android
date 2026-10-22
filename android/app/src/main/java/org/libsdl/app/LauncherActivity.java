package org.libsdl.app;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.provider.Settings;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class LauncherActivity extends Activity {
    private static final int REQUEST_GAME_FOLDER = 4101;
    private static final String PREFS = "pes2013_launcher";
    private static final String GAME_PATH = "game_path";
    private static final String PICKED_PATH = "picked_path";
    private static final int REQUEST_STORAGE = 4102;
    private static final String CHANNEL_URL = "https://youtube.com/@shisui-bahiano?si=O5-QfhdS449-ReXz";

    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private TextView status;
    private Button playButton;
    private ProgressBar progress;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        SessionLogger.start(this, "LauncherActivity");
        SessionLogger.log(this, "LAUNCHER_CREATED");
        buildUi();
        refreshState();
    }

    private void buildUi() {
        ScrollView scroll = new ScrollView(this);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(28, 32, 28, 28);
        root.setBackgroundColor(Color.rgb(12, 15, 21));

        TextView title = label("PES 2013", 30, 0xffffffff);
        title.setTypeface(null, android.graphics.Typeface.BOLD);
        root.addView(title, params(0, 0, 0, 0));

        TextView subtitle = label("Recompilação nativa para Android", 15, 0xffaeb8c8);
        root.addView(subtitle, params(0, 5, 0, 0));

        LinearLayout gameCard = card();
        TextView cardTitle = label("DADOS DO JOGO", 12, 0xff78a9ff);
        cardTitle.setTypeface(null, android.graphics.Typeface.BOLD);
        gameCard.addView(cardTitle);
        status = label("Nenhuma pasta selecionada.", 14, 0xffe2e7ef);
        status.setMaxLines(4);
        gameCard.addView(status, params(0, 12, 0, 0));
        Button choose = new Button(this);
        choose.setText("SELECIONAR PASTA");
        choose.setOnClickListener(v -> chooseFolder());
        gameCard.addView(choose, params(-1, 16, 0, 0));
        root.addView(gameCard, params(-1, 28, 0, 0));

        playButton = new Button(this);
        playButton.setText("▶  PLAY");
        playButton.setTextSize(18);
        playButton.setOnClickListener(v -> launchGame());
        root.addView(playButton, params(-1, 0, 0, 0));

        LinearLayout logCard = card();
        TextView logTitle = label("DIAGNÓSTICO", 12, 0xff78a9ff);
        logTitle.setTypeface(null, android.graphics.Typeface.BOLD);
        logCard.addView(logTitle);
        Switch autoLog = new Switch(this);
        autoLog.setText("Gerar log automaticamente");
        autoLog.setTextColor(0xffe2e7ef);
        autoLog.setTextSize(14);
        autoLog.setChecked(SessionLogger.isEnabled(this));
        autoLog.setOnCheckedChangeListener((button, checked) -> {
            SessionLogger.setEnabled(this, checked);
            if (checked) SessionLogger.log(this, "AUTO_LOG_ENABLED");
        });
        logCard.addView(autoLog, params(-1, 8, 0, 0));
        Button exportLog = new Button(this);
        exportLog.setText("MOSTRAR LOCAL DO LOG");
        exportLog.setOnClickListener(v -> showLogLocation());
        logCard.addView(exportLog, params(-1, 4, 0, 0));
        root.addView(logCard, params(-1, 18, 0, 0));

        progress = new ProgressBar(this);
        progress.setVisibility(View.GONE);
        root.addView(progress, params(-2, 16, 0, 0));

        TextView credit = label("Créditos: Shisui Bahiano", 13, 0xff8491a5);
        credit.setGravity(Gravity.CENTER);
        credit.setOnClickListener(v -> {
            try { startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(CHANNEL_URL))); }
            catch (Exception ignored) { }
        });
        root.addView(credit, params(-1, 26, 0, 0));

        scroll.addView(root);
        setContentView(scroll);
    }

    private LinearLayout card() {
        LinearLayout view = new LinearLayout(this);
        view.setOrientation(LinearLayout.VERTICAL);
        view.setPadding(18, 18, 18, 18);
        GradientDrawable background = new GradientDrawable();
        background.setColor(0xff1a202b);
        background.setCornerRadius(18);
        view.setBackground(background);
        return view;
    }

    private LinearLayout.LayoutParams params(int width, int top, int right, int bottom) {
        LinearLayout.LayoutParams value = new LinearLayout.LayoutParams(width == 0 ? -2 : width, -2);
        value.topMargin = top;
        value.rightMargin = right;
        value.bottomMargin = bottom;
        return value;
    }

    private TextView label(String text, float size, int color) {
        TextView view = new TextView(this);
        view.setText(text);
        view.setTextSize(size);
        view.setTextColor(color);
        return view;
    }

    private void refreshState() {
        String path = getSharedPreferences(PREFS, MODE_PRIVATE).getString(GAME_PATH, null);
        boolean ready = path != null && new File(path).isDirectory();
        playButton.setEnabled(ready);
        if (ready) status.setText("Pasta pronta:\n" + path);
    }

    private void chooseFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION |
                Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION |
                Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);
        startActivityForResult(intent, REQUEST_GAME_FOLDER);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_GAME_FOLDER || resultCode != RESULT_OK || data == null || data.getData() == null) return;
        Uri tree = data.getData();
        try {
            getContentResolver().takePersistableUriPermission(tree,
                    data.getFlags() & Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException ignored) { }
        importFolder(tree);
    }

    /** Usa a pasta escolhida diretamente (sem copiar). Exige acesso a todos os arquivos. */
    private void importFolder(Uri tree) {
        SessionLogger.log(this, "FOLDER_PICKED uri=" + tree);
        File dir = resolveTreePath(DocumentsContract.getTreeDocumentId(tree));
        if (dir == null) {
            status.setText("Não foi possível obter o caminho real da pasta.\nEscolha uma pasta do armazenamento interno ou do cartão SD.");
            playButton.setEnabled(false);
            return;
        }
        getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString(PICKED_PATH, dir.getAbsolutePath()).apply();
        scanPickedFolder();
    }

    private boolean hasStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) return Environment.isExternalStorageManager();
        return checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    private void requestStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) {
            Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                    Uri.parse("package:" + getPackageName()));
            try { startActivity(intent); }
            catch (Exception e) { startActivity(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)); }
        } else {
            requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE }, REQUEST_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int code, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(code, permissions, results);
        scanPickedFolder();
    }

    @Override protected void onResume() {
        super.onResume();
        if (status != null) scanPickedFolder();
    }

    /** Procura default.xex na pasta escolhida (até 4 níveis) e grava a raiz do jogo. */
    private void scanPickedFolder() {
        String picked = getSharedPreferences(PREFS, MODE_PRIVATE).getString(PICKED_PATH, null);
        if (picked == null) { refreshState(); return; }
        if (!hasStorageAccess()) {
            playButton.setEnabled(false);
            status.setText("Pasta escolhida:\n" + picked + "\n\nConceda \"Acesso a todos os arquivos\" ao app para ler o jogo direto da pasta.");
            requestStorageAccess();
            return;
        }
        progress.setVisibility(View.VISIBLE);
        executor.execute(() -> {
            File gameRoot = findGameRoot(new File(picked), 0);
            runOnUiThread(() -> {
                progress.setVisibility(View.GONE);
                if (gameRoot == null) {
                    getSharedPreferences(PREFS, MODE_PRIVATE).edit().remove(GAME_PATH).apply();
                    SessionLogger.log(this, "XEX_NOT_FOUND picked=" + picked);
                    status.setText("default.xex não encontrado em:\n" + picked);
                    playButton.setEnabled(false);
                } else {
                    getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString(GAME_PATH, gameRoot.getAbsolutePath()).apply();
                    SessionLogger.log(this, "GAME_ROOT_READY gameRoot=" + gameRoot.getAbsolutePath());
                    refreshState();
                }
            });
        });
    }

    /** Converte o id de documento SAF ("primary:Pasta/Jogo" ou "XXXX-XXXX:Pasta") em caminho POSIX. */
    private File resolveTreePath(String docId) {
        int colon = docId.indexOf(':');
        if (colon < 0) return null;
        String volume = docId.substring(0, colon);
        String rel = docId.substring(colon + 1);
        File base = volume.equalsIgnoreCase("primary")
                ? Environment.getExternalStorageDirectory()
                : new File("/storage/" + volume);
        return rel.isEmpty() ? base : new File(base, rel);
    }

    private File findGameRoot(File directory, int depth) {
        if (depth > 4 || directory == null || !directory.isDirectory()) return null;
        File[] entries = directory.listFiles();
        if (entries == null) return null;
        for (File entry : entries) {
            if (entry.isFile() && entry.getName().equalsIgnoreCase("default.xex")) return directory;
        }
        for (File entry : entries) {
            if (entry.isDirectory()) {
                File result = findGameRoot(entry, depth + 1);
                if (result != null) return result;
            }
        }
        return null;
    }

    private void launchGame() {
        String path = getSharedPreferences(PREFS, MODE_PRIVATE).getString(GAME_PATH, null);
        if (path == null || !new File(path).isDirectory()) return;
        SessionLogger.log(this, "PLAY_REQUEST gameRoot=" + path);
        startActivity(new Intent(this, GameActivity.class));
    }

    private void showLogLocation() {
        File latest = SessionLogger.latestFile(this);
        String path = latest == null ? "Nenhum log gerado ainda" : latest.getAbsolutePath();
        Toast.makeText(this, path, Toast.LENGTH_LONG).show();
    }

    @Override protected void onDestroy() {
        executor.shutdownNow();
        SessionLogger.close(this, "LauncherActivity");
        super.onDestroy();
    }
}
