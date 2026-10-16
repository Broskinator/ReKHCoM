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
    private static final String[] SAVES={"kh_com.sav","kh_com_us.sav","kh_com_jp.sav"};
    private Spinner region,language,zoom;
    private TextView status;
    private Button play;
    private int saveRegion;
    private boolean busy;
    private String message="";
    private int dp(int n){return (int)(n*getResources().getDisplayMetrics().density+.5f);}
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        ScrollView scroll=new ScrollView(this);
        LinearLayout layout=new LinearLayout(this);layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(dp(24),dp(32),dp(24),dp(24));layout.setBackgroundColor(Color.rgb(17,22,32));
        scroll.addView(layout);setContentView(scroll);
        TextView title=new TextView(this);title.setText("ReKHCoM · Android alpha");title.setTextSize(26);title.setTextColor(Color.WHITE);layout.addView(title);
        TextView info=new TextView(this);info.setText("Pruebas Android · Texto original · Sin ROM incluida\nInstala tu volcado europeo, americano o japonés.");info.setTextColor(Color.LTGRAY);info.setPadding(0,dp(12),0,dp(24));layout.addView(info);
        region=spinner(layout,"Versión",new String[]{"Europa","América","Japón"});
        language=spinner(layout,"Idioma (Europa)",new String[]{"English","Français","Deutsch","Italiano","Español"});
        language.setSelection(4);
        zoom=spinner(layout,"Zoom",new String[]{"x1 · Exigente","x2","x3","x4","x5","x6"});zoom.setSelection(3);
        Button rom=button(layout,"Cargar ROM desde Archivos");rom.setOnClickListener(v->pick(10));
        play=button(layout,"Jugar");play.setOnClickListener(v->launch());
        Button saves=button(layout,"Importar partida · crea copia previa");saves.setOnClickListener(v->{saveRegion=region.getSelectedItemPosition();pick(20);});
        Button export=button(layout,"Exportar partida");export.setOnClickListener(v->{saveRegion=region.getSelectedItemPosition();Intent i=new Intent(Intent.ACTION_CREATE_DOCUMENT);i.setType("application/octet-stream");i.addCategory(Intent.CATEGORY_OPENABLE);i.putExtra(Intent.EXTRA_TITLE,SAVES[saveRegion]);startActivityForResult(i,30);});
        status=new TextView(this);status.setTextColor(Color.rgb(180,207,240));status.setPadding(0,dp(20),0,dp(20));layout.addView(status);
        TextView controls=new TextView(this);controls.setText("Controles: cruceta, A/B y L/R. Rodar y apilar tienen botones propios. START + SELECT abre las opciones.\n\nVersión experimental: sin estados rápidos ni texto HD. Las partidas normales se guardan en el almacenamiento privado de la app.");controls.setTextColor(Color.LTGRAY);layout.addView(controls);
        region.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener(){public void onItemSelected(android.widget.AdapterView<?> a,View v,int p,long id){refresh();}public void onNothingSelected(android.widget.AdapterView<?> a){}});
        refresh();
    }
    private Spinner spinner(LinearLayout p,String name,String[] values){
        TextView label=new TextView(this);label.setText(name);label.setTextColor(Color.WHITE);p.addView(label);
        Spinner s=new Spinner(this);s.setAdapter(new ArrayAdapter<String>(this,android.R.layout.simple_spinner_dropdown_item,values));p.addView(s,new LinearLayout.LayoutParams(-1,dp(56)));return s;
    }
    private Button button(LinearLayout p,String label){Button b=new Button(this);b.setText(label);p.addView(b,new LinearLayout.LayoutParams(-1,dp(58)));return b;}
    private File rom(int index){return new File(getFilesDir(),"roms/"+CODES[index]+".gba");}
    private void refresh(){if(status==null)return;int n=region.getSelectedItemPosition();boolean present=rom(n).isFile();play.setEnabled(present&&!busy);status.setText((message.isEmpty()?"":message+"\n")+(present?"ROM verificada: "+CODES[n]:"Instala una ROM para esta versión."));}
    private void pick(int code){if(busy)return;Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);i.setType("*/*");i.addCategory(Intent.CATEGORY_OPENABLE);i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);startActivityForResult(i,code);}
    @Override protected void onActivityResult(int request,int result,Intent data){
        super.onActivityResult(request,result,data);if(result!=RESULT_OK||data==null||data.getData()==null)return;
        Uri uri=data.getData();busy=true;refresh();
        final int selectedSaveRegion=saveRegion;
        new Thread(()->{
            String msg;
            try {
                if(request==10)msg=importRom(uri);
                else if(request==20){importSave(uri,selectedSaveRegion);msg="Partida importada. Copia previa conservada si existía.";}
                else if(request==30){exportSave(uri,selectedSaveRegion);msg="Partida exportada.";}
                else msg="Operación desconocida.";
            }catch(Exception e){msg="No se pudo completar: "+e.getMessage();}
            final String end=msg;runOnUiThread(()->{busy=false;message=end;refresh();});
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
                final int selected=n;runOnUiThread(()->region.setSelection(selected));return "ROM instalada y verificada: "+CODES[n];
            }
            throw new IOException("ROM no reconocida. Usa tu volcado EU, US o JP admitido.");
        }finally{if(temp.exists())temp.delete();}
    }
    private void importSave(Uri uri,int selectedSaveRegion)throws Exception{
        File temp=File.createTempFile("save-",".tmp",getFilesDir());
        try {
            try(InputStream in=getContentResolver().openInputStream(uri);FileOutputStream out=new FileOutputStream(temp)){if(in==null)throw new IOException("Archivo no accesible");copy(in,out,65536);out.getFD().sync();}
            if(temp.length()!=65536)throw new IOException("Partida SRAM de 64 KiB requerida");
            File target=new File(getFilesDir(),SAVES[selectedSaveRegion]);
            if(target.exists())try(FileInputStream in=new FileInputStream(target);FileOutputStream out=new FileOutputStream(new File(getFilesDir(),target.getName()+".backup-"+System.currentTimeMillis()))){copy(in,out,65536);out.getFD().sync();}
            if(!temp.renameTo(target))throw new IOException("No se pudo guardar la partida");
        }finally{if(temp.exists())temp.delete();}
    }
    private void exportSave(Uri uri,int selectedSaveRegion)throws Exception{
        File source=new File(getFilesDir(),SAVES[selectedSaveRegion]);if(!source.isFile())throw new IOException("Todavía no hay una partida guardada");
        try(FileInputStream in=new FileInputStream(source);OutputStream out=getContentResolver().openOutputStream(uri,"w")){if(out==null)throw new IOException("Destino no accesible");copy(in,out,65536);}
    }
    private void launch(){
        int n=region.getSelectedItemPosition();if(!rom(n).isFile()||busy)return;
        File config=new File(getFilesDir(),"kh_com.ini");
        // Change only launcher-owned fields; preserve in-game audio/controls.
        try {
            java.util.Properties properties=new java.util.Properties();
            if(config.isFile())try(FileInputStream in=new FileInputStream(config)){properties.load(in);}
            properties.setProperty("config_version","11");properties.setProperty("view_mode","4");
            properties.setProperty("zoom",Integer.toString((zoom.getSelectedItemPosition()+1)*2));
            properties.setProperty("hd_text","0");properties.setProperty("narrow_fit","1");
            File temp=File.createTempFile("config-",".tmp",getFilesDir());
            try(FileOutputStream out=new FileOutputStream(temp)){properties.store(out,"ReKHCoM Android alpha");out.getFD().sync();}
            if(!temp.renameTo(config))throw new IOException("No se pudo escribir la configuración");
            String[] langs={"en","fr","de","it","es"};
            Intent i=new Intent(this,GameActivity.class);i.putExtra("region",REGIONS[n]);i.putExtra("rom",rom(n).getAbsolutePath());i.putExtra("language",langs[language.getSelectedItemPosition()]);startActivity(i);
        }catch(Exception e){message=e.getMessage();refresh();}
    }
}
