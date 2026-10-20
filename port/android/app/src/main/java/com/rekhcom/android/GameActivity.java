package com.rekhcom.android;

import org.libsdl.app.SDLActivity;

/** Runs the selected native core in :game, separate from the ROM launcher. */
public class GameActivity extends SDLActivity {
    private String region() {
        String r=getIntent().getStringExtra("region");
        return "us".equals(r)||"jp".equals(r)?r:"eu";
    }
    @Override protected String[] getLibraries() { return new String[]{"SDL3","rekhcom_"+region()}; }
    /** Test app only (application id ending in ".smoke"): KEY=VALUE lines of files/smoke_env.txt become
     *  environment variables, so one APK runs many harness scenarios. The normal app never reads them. */
    private void smokeEnvironment() {
        if (!getPackageName().endsWith(".smoke")) return;
        try {
            java.io.File file=new java.io.File(getFilesDir(),"smoke_env.txt");
            if (!file.exists()) return;
            java.io.ByteArrayOutputStream bytes=new java.io.ByteArrayOutputStream();
            try (java.io.FileInputStream in=new java.io.FileInputStream(file)) {
                byte[] chunk=new byte[4096];
                for (int n; (n=in.read(chunk))>0;) bytes.write(chunk,0,n);
            }
            for (String line: bytes.toString("UTF-8").split("\r?\n")) {
                int eq=line.indexOf('=');
                if (eq>0) nativeSetenv(line.substring(0,eq).trim(),line.substring(eq+1).trim());
            }
        } catch (Exception e) { android.util.Log.w("ReKHCoM","smoke env: "+e); }
    }
    @Override protected String[] getArguments() {
        smokeEnvironment();
        /* No --language: the European cartridge asks for it itself. */
        return new String[]{"--data-dir",getFilesDir().getAbsolutePath(),"--rom",
            getIntent().getStringExtra("rom")};
    }
}
