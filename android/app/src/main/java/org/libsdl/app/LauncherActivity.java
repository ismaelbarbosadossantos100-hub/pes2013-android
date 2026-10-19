package org.libsdl.app;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class LauncherActivity extends Activity {
    private static final int REQUEST_GAME_FOLDER = 4101;
    private static final String PREFS = "pes2013_launcher";
    private static final String GAME_PATH = "game_path";
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

    private void importFolder(Uri tree) {
        SessionLogger.log(this, "IMPORT_START uri=" + tree);
        playButton.setEnabled(false);
        progress.setVisibility(View.VISIBLE);
        status.setText("Copiando a pasta privada do app...\nIsso pode levar alguns minutos.");
        executor.execute(() -> {
            File destination = new File(getExternalFilesDir(null), "game_data");
            try {
                deleteChildren(destination);
                if (!destination.mkdirs() && !destination.isDirectory()) throw new IOException("Não foi possível criar a pasta do jogo");
                copyChildren(tree, DocumentsContract.getTreeDocumentId(tree), destination);
                File gameRoot = findGameRoot(destination, 0);
                if (gameRoot == null) {
                    throw new IOException("default.xex não encontrado na pasta selecionada");
                }
                getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString(GAME_PATH, gameRoot.getAbsolutePath()).apply();
                SessionLogger.log(this, "IMPORT_READY gameRoot=" + gameRoot.getAbsolutePath());
                runOnUiThread(() -> { progress.setVisibility(View.GONE); refreshState(); });
            } catch (Exception error) {
                SessionLogger.append(error);
                runOnUiThread(() -> {
                    progress.setVisibility(View.GONE);
                    status.setText("Falha ao importar a pasta:\n" + error.getMessage());
                    playButton.setEnabled(false);
                });
            }
        });
    }

    private void copyChildren(Uri tree, String parentId, File destination) throws IOException {
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(tree, parentId);
        String[] projection = { DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE };
        try (Cursor cursor = getContentResolver().query(children, projection, null, null, null)) {
            if (cursor == null) throw new IOException("Não foi possível ler a pasta selecionada");
            while (cursor.moveToNext()) {
                String id = cursor.getString(0);
                String name = cursor.getString(1);
                String mime = cursor.getString(2);
                if (name == null || name.isEmpty()) name = id;
                File output = new File(destination, safeName(name));
                Uri document = DocumentsContract.buildDocumentUriUsingTree(tree, id);
                if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) {
                    if (!output.mkdirs() && !output.isDirectory()) throw new IOException("Falha criando " + output);
                    copyChildren(tree, id, output);
                } else {
                    copyFile(document, output);
                }
            }
        }
    }

    private void copyFile(Uri source, File output) throws IOException {
        File parent = output.getParentFile();
        if (parent != null) parent.mkdirs();
        try (InputStream input = getContentResolver().openInputStream(source);
             FileOutputStream out = new FileOutputStream(output)) {
            if (input == null) throw new IOException("Não foi possível abrir " + source);
            byte[] buffer = new byte[1024 * 1024];
            int count;
            while ((count = input.read(buffer)) != -1) out.write(buffer, 0, count);
        }
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

    private String safeName(String name) {
        return name.replace('/', '_').replace('\\', '_');
    }

    private void deleteChildren(File directory) {
        if (!directory.exists()) return;
        File[] children = directory.listFiles();
        if (children != null) for (File child : children) {
            if (child.isDirectory()) deleteChildren(child);
            child.delete();
        }
        directory.delete();
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
