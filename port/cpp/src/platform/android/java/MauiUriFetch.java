// dev.mauicpp.MauiUriFetch — the http(s) byte fetch behind the Android backend's uri image source
// (the C++ analog of what MAUI reaches through Glide: UriImageSourceService.Android.cs calls
// PlatformInterop.LoadImageFromUri, whose Java half downloads the uri OFF the UI thread and delivers
// the decoded result BACK ON it). The port carries no Glide AAR, so this is the same shape written
// against the stock framework: java.net.HttpURLConnection on a worker thread, then a
// Handler(Looper.getMainLooper()) post so the native sink — which decodes the bytes and pushes the
// Bitmap into the ImageView — runs on the UI thread, exactly where a View may be touched.
//
// The peer is a heap-owned native uri_fetch_state* (the loader's byte sink + its cancellation token)
// that src/platform/android/image_handler.cpp hands over; nativeUriFetched takes ownership back and
// frees it, so every fetch delivers exactly once (a null/empty payload on any failure — the loader
// treats empty bytes as "nothing loaded", the same degradation as a missing local file).
//
// nativeUriFetched is bound from C++ via JNIEnv.RegisterNatives before the first fetch is dispatched
// (reflection-free, no Java_* symbol export needed) — the MauiDialogBridge recipe.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library: runtime classes the native backend needs in the process' dex. The widget test host
// and both gallery app hosts glob every *.java here, so this file is picked up automatically.
package dev.mauicpp;

import android.os.Handler;
import android.os.Looper;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public final class MauiUriFetch {
    // Timeouts so a dead host cannot pin the worker thread forever; the sink then delivers empty bytes.
    private static final int TIMEOUT_MS = 15000;
    // HttpURLConnection follows same-protocol redirects itself but REFUSES http->https (and back), which
    // is exactly the hop a short-link like https://aka.ms/campus.jpg can take. Follow those by hand.
    private static final int MAX_REDIRECTS = 5;
    private static final int CHUNK = 64 * 1024;

    private MauiUriFetch() {
    }

    // Called from C++ (image_handler.cpp's fetch_uri_async) on the UI thread. Returns IMMEDIATELY; the
    // download runs on a worker and its result is posted back to the main looper.
    public static void fetch(final long peer, final String url) {
        final Handler main = new Handler(Looper.getMainLooper());
        new Thread(new Runnable() {
            @Override
            public void run() {
                final byte[] data = download(url);
                main.post(new Runnable() {
                    @Override
                    public void run() {
                        nativeUriFetched(peer, data);
                    }
                });
            }
        }, "maui-uri-fetch").start();
    }

    // GET `url` into a byte[], following up to MAX_REDIRECTS Location hops. null on any failure (a
    // malformed uri, a non-200 status, an I/O error) — the native side turns that into empty bytes.
    private static byte[] download(String url) {
        String next = url;
        for (int hop = 0; hop <= MAX_REDIRECTS; ++hop) {
            HttpURLConnection connection = null;
            try {
                connection = (HttpURLConnection) new URL(next).openConnection();
                connection.setConnectTimeout(TIMEOUT_MS);
                connection.setReadTimeout(TIMEOUT_MS);
                connection.setInstanceFollowRedirects(true);
                final int status = connection.getResponseCode();
                if (status == HttpURLConnection.HTTP_MOVED_PERM
                        || status == HttpURLConnection.HTTP_MOVED_TEMP
                        || status == HttpURLConnection.HTTP_SEE_OTHER
                        || status == 307
                        || status == 308) {
                    // A cross-protocol redirect the connection would not follow on its own; resolve the
                    // Location against the current url (it may be relative) and retry.
                    final String location = connection.getHeaderField("Location");
                    if (location == null || location.isEmpty()) {
                        return null;
                    }
                    next = new URL(new URL(next), location).toString();
                    continue;
                }
                if (status != HttpURLConnection.HTTP_OK) {
                    return null;
                }
                return drain(connection.getInputStream());
            } catch (Throwable t) {
                return null;
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        }
        return null; // redirect budget exhausted
    }

    private static byte[] drain(InputStream stream) throws java.io.IOException {
        try {
            final ByteArrayOutputStream out = new ByteArrayOutputStream();
            final byte[] buffer = new byte[CHUNK];
            for (int n = stream.read(buffer); n > 0; n = stream.read(buffer)) {
                out.write(buffer, 0, n);
            }
            return out.size() > 0 ? out.toByteArray() : null;
        } finally {
            stream.close();
        }
    }

    // Bound from C++ via JNIEnv.RegisterNatives before the first fetch is dispatched. `peer` is the
    // native uri_fetch_state*; the native side takes ownership back and frees it. `data` is null when
    // the download failed.
    private static native void nativeUriFetched(long peer, byte[] data);
}
