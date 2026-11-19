/*
 * Zuma HD for Android
 *
 * Project Owner & Creator: Muhamed Alzarqi
 * AI Programming Assistant: Claude (Anthropic)
 * Created through human-AI collaboration
 *
 * Based on Zuma Deluxe HD by GalaxyShad and s4lat.
 */
package com.muhamed.zumahd;

import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class ZumaActivity extends SDLActivity {
    private static final String TAG = "ZumaHD";
    private static final String[] CONTENT_FOLDERS = {
        "fonts", "images", "levels", "music", "sounds"
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // The game reads its files from normal folders, so copy them out
        // of the APK once (and again after every app update).
        copyContentIfNeeded();

        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        requestSixtyHz();
    }

    /** Ask the screen to run at 60 Hz so the game's 60 FPS is smooth. */
    private void requestSixtyHz() {
        try {
            WindowManager.LayoutParams lp = getWindow().getAttributes();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                Display display = getWindowManager().getDefaultDisplay();
                Display.Mode current = display.getMode();
                Display.Mode best = null;
                for (Display.Mode m : display.getSupportedModes()) {
                    if (m.getPhysicalWidth() != current.getPhysicalWidth()
                            || m.getPhysicalHeight() != current.getPhysicalHeight())
                        continue;
                    if (Math.abs(m.getRefreshRate() - 60f) < 1f) {
                        best = m;
                        break;
                    }
                }
                if (best != null)
                    lp.preferredDisplayModeId = best.getModeId();
            }
            lp.preferredRefreshRate = 60f;
            getWindow().setAttributes(lp);
        } catch (Exception e) {
            Log.w(TAG, "Could not request 60 Hz", e);
        }
    }

    private void copyContentIfNeeded() {
        File dir = getFilesDir();
        File marker = new File(dir, ".content_version");
        String version;
        try {
            version = String.valueOf(
                getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime);
        } catch (Exception e) {
            version = "unknown";
        }

        if (marker.exists() && version.equals(readText(marker)))
            return;

        try {
            AssetManager assets = getAssets();
            for (String folder : CONTENT_FOLDERS)
                copyAssetTree(assets, folder, new File(dir, folder));
            writeText(marker, version);
        } catch (Exception e) {
            Log.e(TAG, "Copying game files failed", e);
        }
    }

    private void copyAssetTree(AssetManager assets, String path, File target) throws Exception {
        String[] children = assets.list(path);
        if (children != null && children.length > 0) {
            target.mkdirs();
            for (String child : children)
                copyAssetTree(assets, path + "/" + child, new File(target, child));
            return;
        }

        target.getParentFile().mkdirs();
        try (InputStream in = assets.open(path);
             OutputStream out = new FileOutputStream(target)) {
            byte[] buf = new byte[64 * 1024];
            int n;
            while ((n = in.read(buf)) > 0)
                out.write(buf, 0, n);
        }
    }

    private static String readText(File f) {
        try (InputStream in = new java.io.FileInputStream(f)) {
            byte[] b = new byte[64];
            int n = in.read(b);
            return n > 0 ? new String(b, 0, n, "UTF-8") : "";
        } catch (Exception e) {
            return "";
        }
    }

    private static void writeText(File f, String s) throws Exception {
        try (OutputStream out = new FileOutputStream(f)) {
            out.write(s.getBytes("UTF-8"));
        }
    }
}
