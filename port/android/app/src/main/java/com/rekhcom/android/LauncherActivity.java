package com.rekhcom.android;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.graphics.Color;
import android.view.View;
import android.widget.*;
import java.io.*;
import java.security.MessageDigest;
import java.util.Locale;

/** ROM-free native launcher. SAF imports are bounded, hashed and atomic. */
public class LauncherActivity extends Activity {
    private static final String[] REGIONS={"eu","us","jp"};
    private static final String[] CODES={"B8CP","B8CE","B8CJ"};
    private static final String[] HASHES={"8db73586cdb11b3795907edebf43228dbcd3e6b2",
        "10729bd884f8fdca7a310b6d606c52e46657aa48","59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa"};
    private Spinner region,zoom,bank;
    private TextView status;
    private Button play;
    private int saveRegion,saveBank=1,zoomFirst;
    private boolean pickerPending;
    private static volatile boolean busy;
    private static volatile String message="";
    private final android.os.Handler refreshHandler=new android.os.Handler(android.os.Looper.getMainLooper());
    private final Runnable refreshOperation=new Runnable(){public void run(){refresh();if(busy)refreshHandler.postDelayed(this,200);}};
    @Override protected void onResume(){super.onResume();refreshHandler.post(refreshOperation);}
    @Override protected void onPause(){refreshHandler.removeCallbacks(refreshOperation);super.onPause();}
    /* Launcher text in Spanish or English (device language unless chosen with the
       small link at the bottom); the game's own language is asked by the game. */
    private boolean english;
    private String t(String es,String en){return english?en:es;}
    private int dp(int n){return (int)(n*getResources().getDisplayMetrics().density+.5f);}
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        String ui=getSharedPreferences("launcher",MODE_PRIVATE).getString("ui_language","");
        english=ui.isEmpty()?!"es".equals(Locale.getDefault().getLanguage()):"en".equals(ui);
        if(state!=null){saveRegion=state.getInt("saveRegion");saveBank=state.getInt("saveBank",1);pickerPending=state.getBoolean("pickerPending");}
        ScrollView scroll=new ScrollView(this);
        LinearLayout layout=new LinearLayout(this);layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(dp(24),dp(32),dp(24),dp(24));layout.setBackgroundColor(Color.rgb(17,22,32));
        scroll.addView(layout);setContentView(scroll);
        TextView title=new TextView(this);title.setText("ReKHCoM · Android alpha");title.setTextSize(26);title.setTextColor(Color.WHITE);layout.addView(title);
        TextView info=new TextView(this);info.setText(t("Pruebas Android · Texto original · Sin ROM incluida\nInstala tu volcado europeo, americano o japonés.","Android test build · Original text · No ROM included\nInstall your European, American or Japanese dump."));info.setTextColor(Color.LTGRAY);info.setPadding(0,dp(12),0,dp(24));layout.addView(info);
        region=spinner(layout,t("Versión","Version"),new String[]{t("Europa","Europe"),t("América","America"),t("Japón","Japan")});
        bank=spinner(layout,t("Banco de partidas","Save bank"),new String[]{"1","2","3","4","5"});
        try {
            java.util.Properties saved=new java.util.Properties();
            File config=new File(getFilesDir(),"kh_com.ini");
            if(config.isFile())try(FileInputStream in=new FileInputStream(config)){saved.load(in);}
            bank.setSelection(Math.max(0,Math.min(4,Integer.parseInt(saved.getProperty("save_bank","1"))-1)));
        }catch(IOException|NumberFormatException ignored){}

        // The game runs landscape and must retain at least 160 source lines.
        // Offer only scales that fit this display, matching PortZoomMin/Max.
        android.util.DisplayMetrics metrics=getResources().getDisplayMetrics();
        int width=Math.max(metrics.widthPixels,metrics.heightPixels);
        int height=Math.min(metrics.widthPixels,metrics.heightPixels);
        zoomFirst=Math.max(1,Math.max((width+2559)/2560,(height+1439)/1440));
        int zoomLast=Math.max(zoomFirst,Math.min(64,height/160));
        String[] scales=new String[zoomLast-zoomFirst+1];
        for(int n=0;n<scales.length;n++){int value=zoomFirst+n;scales[n]="x"+value+(value==1?t(" · Exigente"," · Demanding"):"");}
        zoom=spinner(layout,"Zoom",scales);
        zoom.setSelection(Math.max(0,Math.min(4,zoomLast)-zoomFirst));
        Button rom=button(layout,t("Cargar ROM desde Archivos","Load ROM from Files"));rom.setOnClickListener(v->pick(10));
        play=button(layout,t("Jugar","Play"));play.setOnClickListener(v->launch());
        TextView saveHelp=new TextView(this);saveHelp.setText(t("Importa guardados normales .sav/.srm del port o emuladores. Elige la misma regi\u00f3n y el banco de destino. Se conserva una copia previa. No admite estados r\u00e1pidos ni convierte entre regiones.","Import normal .sav/.srm saves from the port or emulators. Choose the same region and the target bank. A previous copy is kept. Save states are not supported and regions are not converted."));saveHelp.setTextColor(Color.LTGRAY);layout.addView(saveHelp);
        Button saves=button(layout,t("Importar .sav / .srm","Import .sav / .srm"));saves.setOnClickListener(v->{if(busy||pickerPending)return;saveRegion=region.getSelectedItemPosition();saveBank=bank.getSelectedItemPosition()+1;pick(20);});
        Button export=button(layout,t("Exportar partida","Export save"));export.setOnClickListener(v->{if(busy||pickerPending)return;saveRegion=region.getSelectedItemPosition();saveBank=bank.getSelectedItemPosition()+1;pickerPending=true;Intent i=new Intent(Intent.ACTION_CREATE_DOCUMENT);i.setType("application/octet-stream");i.addCategory(Intent.CATEGORY_OPENABLE);i.putExtra(Intent.EXTRA_TITLE,SaveImport.bankName(saveRegion,saveBank));startActivityForResult(i,30);});
        status=new TextView(this);status.setTextColor(Color.rgb(180,207,240));status.setPadding(0,dp(20),0,dp(20));layout.addView(status);
        TextView controls=new TextView(this);controls.setText(t("Controles: cruceta, A/B y L/R. Rodar y apilar tienen botones propios. El botón … o START + SELECT abre las opciones.\n\nVersión experimental: sin estados rápidos ni texto HD. Las partidas normales se guardan en el almacenamiento privado de la app.","Controls: d-pad, A/B and L/R. Dodge roll and stock have their own buttons. The … button or START + SELECT opens the options.\n\nExperimental version: no save states or HD text. Normal saves are kept in the app's private storage."));controls.setTextColor(Color.LTGRAY);layout.addView(controls);
        TextView uiLanguage=new TextView(this);uiLanguage.setText(t("Idioma de la interfaz: Español","Interface language: English"));uiLanguage.setTextColor(Color.GRAY);uiLanguage.setTextSize(12);uiLanguage.setPadding(0,dp(28),0,0);layout.addView(uiLanguage);
        uiLanguage.setOnClickListener(v->{getSharedPreferences("launcher",MODE_PRIVATE).edit().putString("ui_language",english?"es":"en").apply();recreate();});
        region.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener(){public void onItemSelected(android.widget.AdapterView<?> a,View v,int p,long id){refresh();}public void onNothingSelected(android.widget.AdapterView<?> a){}});
        if(state!=null){region.setSelection(state.getInt("region"));bank.setSelection(state.getInt("bank"));zoom.setSelection(Math.max(0,Math.min(scales.length-1,state.getInt("zoom"))));}
        refresh();
    }
    @Override protected void onSaveInstanceState(Bundle state){super.onSaveInstanceState(state);state.putInt("region",region.getSelectedItemPosition());state.putInt("bank",bank.getSelectedItemPosition());state.putInt("zoom",zoom.getSelectedItemPosition());state.putInt("saveRegion",saveRegion);state.putInt("saveBank",saveBank);state.putBoolean("pickerPending",pickerPending);}
    private Spinner spinner(LinearLayout p,String name,String[] values){
        TextView label=new TextView(this);label.setText(name);label.setTextColor(Color.WHITE);p.addView(label);
        Spinner s=new Spinner(this);s.setAdapter(new ArrayAdapter<String>(this,android.R.layout.simple_spinner_dropdown_item,values));p.addView(s,new LinearLayout.LayoutParams(-1,dp(56)));return s;
    }
    private Button button(LinearLayout p,String label){Button b=new Button(this);b.setText(label);p.addView(b,new LinearLayout.LayoutParams(-1,dp(58)));return b;}
    private File rom(int index){return new File(getFilesDir(),"roms/"+CODES[index]+".gba");}
    private void refresh(){if(status==null)return;int n=region.getSelectedItemPosition();boolean present=rom(n).isFile();play.setEnabled(present&&!busy&&!pickerPending);status.setText((message.isEmpty()?"":message+"\n")+(present?t("ROM verificada: ","ROM verified: ")+CODES[n]:t("Instala una ROM para esta versión.","Install a ROM for this version.")));}
    private void pick(int code){if(busy||pickerPending)return;pickerPending=true;Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);i.setType("*/*");i.addCategory(Intent.CATEGORY_OPENABLE);i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);startActivityForResult(i,code);}
    @Override protected void onActivityResult(int request,int result,Intent data){
        super.onActivityResult(request,result,data);pickerPending=false;if(result!=RESULT_OK||data==null||data.getData()==null)return;
        Uri uri=data.getData();busy=true;refresh();
        final int selectedSaveRegion=saveRegion;
        final int selectedBank=saveBank;
        new Thread(()->{
            String msg;
            try {
                if(request==10)msg=importRom(uri);
                else if(request==20){importSave(uri,selectedSaveRegion,selectedBank);msg=t("Partida importada. Copia previa conservada si existía.","Save imported. The previous copy was kept if there was one.");}
                else if(request==30){exportSave(uri,selectedSaveRegion,selectedBank);msg=t("Partida exportada.","Save exported.");}
                else msg=t("Operación desconocida.","Unknown operation.");
            }catch(Exception e){msg=t("No se pudo completar: ","Could not complete: ")+e.getMessage();}
            message=msg;busy=false;runOnUiThread(()->{if(!isDestroyed())refresh();});
        },"ROM-import").start();
    }
    private static void copy(InputStream in,OutputStream out,long limit)throws Exception{
        byte[] buf=new byte[65536];long count=0;int n;
        while((n=in.read(buf))!=-1){if(count+n>limit)throw new IOException("Archivo demasiado grande");out.write(buf,0,n);count+=n;}
    }
    private String importRom(Uri uri)throws Exception{
        File dir=new File(getFilesDir(),"roms");if(!dir.exists()&&!dir.mkdirs())throw new IOException("No hay carpeta de ROM");
        File temp=File.createTempFile("import-",".tmp",dir);
        try {
            try(InputStream in=getContentResolver().openInputStream(uri);FileOutputStream out=new FileOutputStream(temp)){
                if(in==null)throw new IOException("Archivo no accesible");copy(in,out,32L*1024*1024);out.getFD().sync();
            }
            if(temp.length()!=32L*1024*1024)throw new IOException("Se necesita un volcado GBA de 32 MiB");
            MessageDigest digest=MessageDigest.getInstance("SHA-1");byte[] block=new byte[65536];
            try(FileInputStream in=new FileInputStream(temp)){int n;while((n=in.read(block))!=-1)digest.update(block,0,n);}
            StringBuilder hex=new StringBuilder();for(byte b:digest.digest())hex.append(String.format(Locale.ROOT,"%02x",b&255));
            for(int n=0;n<HASHES.length;n++)if(HASHES[n].equals(hex.toString())){
                if(!temp.renameTo(rom(n)))throw new IOException("No se pudo instalar la ROM");
                final int selected=n;runOnUiThread(()->region.setSelection(selected));return t("ROM instalada y verificada: ","ROM installed and verified: ")+CODES[n];
            }
            throw new IOException("ROM no reconocida. Usa tu volcado EU, US o JP admitido.");
        }finally{if(temp.exists())temp.delete();}
    }
    private void importSave(Uri uri,int selectedSaveRegion,int selectedBank)throws Exception{
        File temp=File.createTempFile("save-",".tmp",getFilesDir());
        try {
            try(InputStream in=getContentResolver().openInputStream(uri);FileOutputStream out=new FileOutputStream(temp)){if(in==null)throw new IOException("Archivo no accesible");
                ByteArrayOutputStream bytes=new ByteArrayOutputStream();copy(in,bytes,131072);
                out.write(SaveImport.normalize(bytes.toByteArray(),selectedSaveRegion));out.getFD().sync();}
            if(temp.length()!=65536)throw new IOException("Partida SRAM de 64 KiB requerida");
            File target=new File(getFilesDir(),SaveImport.bankName(selectedSaveRegion,selectedBank));
            if(target.exists())try(FileInputStream in=new FileInputStream(target);FileOutputStream out=new FileOutputStream(new File(getFilesDir(),target.getName()+".backup-"+System.currentTimeMillis()))){copy(in,out,65536);out.getFD().sync();}
            if(!temp.renameTo(target))throw new IOException("No se pudo guardar la partida");
        }finally{if(temp.exists())temp.delete();}
    }
    private void exportSave(Uri uri,int selectedSaveRegion,int selectedBank)throws Exception{
        File source=new File(getFilesDir(),SaveImport.bankName(selectedSaveRegion,selectedBank));if(!source.isFile())throw new IOException("Todavía no hay una partida guardada");
        try(FileInputStream in=new FileInputStream(source);OutputStream out=getContentResolver().openOutputStream(uri,"w")){if(out==null)throw new IOException("Destino no accesible");copy(in,out,65536);}
    }
    private void launch(){
        int n=region.getSelectedItemPosition();if(!rom(n).isFile()||busy||pickerPending)return;
        File config=new File(getFilesDir(),"kh_com.ini");
        // Change only launcher-owned fields; preserve in-game audio/controls.
        try {
            java.util.Properties properties=new java.util.Properties();
            if(config.isFile())try(FileInputStream in=new FileInputStream(config)){properties.load(in);}
            properties.setProperty("config_version","11");properties.setProperty("view_mode","4");
            properties.setProperty("zoom",Integer.toString((zoom.getSelectedItemPosition()+zoomFirst)*2));
            properties.setProperty("save_bank",Integer.toString(bank.getSelectedItemPosition()+1));
            properties.setProperty("narrow_fit","1");
            File temp=File.createTempFile("config-",".tmp",getFilesDir());
            try(FileOutputStream out=new FileOutputStream(temp)){properties.store(out,"ReKHCoM Android alpha");out.getFD().sync();}
            if(!temp.renameTo(config))throw new IOException("No se pudo escribir la configuración");
            Intent i=new Intent(this,GameActivity.class);i.putExtra("region",REGIONS[n]);i.putExtra("rom",rom(n).getAbsolutePath());startActivity(i);
        }catch(Exception e){message=e.getMessage();refresh();}
    }
}
