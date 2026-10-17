"""Exercise the production Java SRAM parser without Android or private saves."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
JDK = next((ROOT.parent / 'toolchain/android/java').glob('jdk-*')) / 'bin'
OUT = ROOT / 'build/android-save-import-test'
OUT.mkdir(parents=True, exist_ok=True)
HARNESS = r'''
import com.rekhcom.android.SaveImport;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;
public class SaveImportTest {
    static int checks;
    static void check(boolean yes) { if (!yes) throw new AssertionError(); checks++; }
    static void block(byte[] b, int offset, int size, int version) {
        byte[] sig=("KHCOM_BACKUP_VER0000001"+version).getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(sig,0,b,offset,sig.length);
        b[offset+24]=b[offset+25]=0;
        long sum=0;
        for(int n=offset;n<offset+size;n+=2) sum+=(b[n]&255)|((b[n+1]&255)<<8);
        sum=(sum&65535)+(sum>>16); int c=(int)~(sum+(sum>>16))&65535;
        b[offset+24]=(byte)c;b[offset+25]=(byte)(c>>8);
    }
    static byte[] save(int region) {
        byte[] b=new byte[65536]; Arrays.fill(b,(byte)255);
        block(b,0x10,0x40,region==0?3:2);
        block(b,0x2f20,0xf14,region==0?3:2); return b;
    }
    static void reject(byte[] b,int r) throws Exception {
        try { SaveImport.normalize(b,r); throw new AssertionError("accepted invalid save"); }
        catch(java.io.IOException expected) { checks++; }
    }
    public static void main(String[] args) throws Exception {
        for(int region=0;region<3;region++) {
            byte[] b=save(region), original=b.clone();
            check(Arrays.equals(b,SaveImport.normalize(b,region)));
            check(Arrays.equals(b,original));
            byte[] small=Arrays.copyOf(b,32768);
            check(Arrays.equals(b,SaveImport.normalize(small,region)));
            byte[] doubled=new byte[131072];
            System.arraycopy(b,0,doubled,0,65536);System.arraycopy(b,0,doubled,65536,65536);
            check(Arrays.equals(b,SaveImport.normalize(doubled,region)));
            doubled[90000]^=1;reject(doubled,region);
            reject(b,region==0?1:0);
            b[0x10+26]^=1;reject(b,region);
            b=save(region);b[0x2f20+26]^=1;reject(b,region);
            // Healthy backup header is enough even if primary is corrupt.
            b=save(region);block(b,0x50,0x40,region==0?3:2);b[0x10+26]^=1;
            check(Arrays.equals(b,SaveImport.normalize(b,region)));
            for(int bank=1;bank<=5;bank++)
                check(SaveImport.bankName(region,bank).endsWith(bank==1?".sav":"_bank"+bank+".sav"));
        }
        reject(new byte[65536],0);reject(new byte[65537],0);reject(new byte[128],0);
        check(SaveImport.bankName(0,1).equals("kh_com.sav"));
        check(SaveImport.bankName(1,3).equals("kh_com_us_bank3.sav"));
        check(SaveImport.bankName(2,5).equals("kh_com_jp_bank5.sav"));
        for(String path:args) {
            byte[] b=java.nio.file.Files.readAllBytes(java.nio.file.Paths.get(path));
            boolean valid=false;
            for(int region=0;region<3;region++) try {
                check(Arrays.equals(b,SaveImport.normalize(b,region)));valid=true;
            } catch(java.io.IOException ignored) {}
            check(valid);
        }
        System.out.println("Android SRAM parser: "+checks+" checks PASS");
    }
}
'''
source = OUT / 'SaveImportTest.java'
source.write_text(HARNESS, encoding='utf-8')
subprocess.run([str(JDK/'javac.exe'), '-encoding', 'UTF-8', '-d', str(OUT),
                str(ROOT/'port/android/app/src/main/java/com/rekhcom/android/SaveImport.java'),
                str(source)], check=True)
import sys
subprocess.run([str(JDK/'java.exe'), '-cp', str(OUT), 'SaveImportTest', *sys.argv[1:]], check=True)
