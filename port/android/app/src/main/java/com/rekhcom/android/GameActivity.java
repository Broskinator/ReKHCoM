package com.rekhcom.android;

import org.libsdl.app.SDLActivity;

/** Runs the selected native core in :game, separate from the ROM launcher. */
public class GameActivity extends SDLActivity {
    private String region() {
        String r=getIntent().getStringExtra("region");
        return "us".equals(r)||"jp".equals(r)?r:"eu";
    }
    @Override protected String[] getLibraries() { return new String[]{"SDL3","rekhcom_"+region()}; }
    @Override protected String[] getArguments() {
        return new String[]{"--data-dir",getFilesDir().getAbsolutePath(),"--rom",
            getIntent().getStringExtra("rom"),"--language",getIntent().getStringExtra("language")};
    }
}
