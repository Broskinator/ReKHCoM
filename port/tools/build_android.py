"""Build the experimental ARMv7 cores and ROM-free Android launcher APK.

Requires the official SDK/NDK/JDK/SDL sources installed under ../toolchain/android.
No emulator/device testing or release upload is implied by a successful build.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import struct
import urllib.request

ROOT=Path(__file__).resolve().parents[2]


def verify_fixed_symbols(path):
    """Absolute cartridge pointers must not receive the ELF load bias.

    Android API24's loader rebases SHN_ABS dynamic relocations. Local binding
    resolves these at link time; reject a library that reintroduces them.
    """
    data=path.read_bytes()
    if data[:6]!=b'\x7fELF\x01\x01':
        raise ValueError(f'Expected little-endian ELF32: {path}')
    start=struct.unpack_from('<I',data,32)[0]
    stride,count=struct.unpack_from('<HH',data,46)
    sections=[struct.unpack_from('<10I',data,start+i*stride) for i in range(count)]
    bad=[]
    for section in sections:
        if section[1] not in (4,9):continue # SHT_RELA / SHT_REL
        symbols=sections[section[6]]
        strings=sections[symbols[6]]
        for at in range(section[4],section[4]+section[5],section[9]):
            info=struct.unpack_from('<I',data,at+4)[0]
            index=info>>8
            if not index:continue
            symbol=symbols[4]+index*symbols[9]
            name=struct.unpack_from('<I',data,symbol)[0]
            shndx=struct.unpack_from('<H',data,symbol+14)[0]
            if shndx==0xfff1:
                begin=strings[4]+name;end=data.index(b'\0',begin)
                bad.append(data[begin:end].decode())
    if bad:raise ValueError('Rebased GBA absolute symbols: '+', '.join(sorted(set(bad))[:12]))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--regions',nargs='+',choices=['eu','us','jp'],default=['eu','us','jp'])
    p.add_argument('--jobs',type=int,default=3)
    p.add_argument('--abi',choices=['armeabi-v7a','x86'],default='armeabi-v7a')
    p.add_argument('--native-only',action='store_true')
    args=p.parse_args()
    if args.jobs<1:p.error('--jobs must be positive')
    tools=ROOT.parent/'toolchain/android';sdk=tools/'sdk'
    ndk=next((tools/'native').glob('android-ndk-*'))
    java=next((tools/'java').glob('jdk-*'))
    sdl=tools/'SDL3-3.4.18';skeleton=sdl/'android-project'
    cmake=sdk/'cmake/3.22.1/bin/cmake.exe'
    ninja=ROOT.parent/'toolchain/bin/ninja.exe'
    suffix='' if args.abi=='armeabi-v7a' else '-'+args.abi
    native=ROOT/('build/android-native'+suffix)
    env=dict(os.environ,JAVA_HOME=str(java),ANDROID_HOME=str(sdk),ANDROID_SDK_ROOT=str(sdk),
             GRADLE_USER_HOME=str(tools/'gradle-cache'),CMAKE_BUILD_PARALLEL_LEVEL=str(args.jobs))
    def run(cmd,cwd=ROOT):subprocess.run([str(x) for x in cmd],cwd=cwd,env=env,check=True)
    for region in args.regions:
        if not (ROOT/f'build/{region}/pc/rom_symbols.s').is_file():
            import sys
            run([sys.executable,'port/tools/gen_pc_assets.py','--version',region])
    run([cmake,'-S',ROOT/'port/android','-B',native,'-G','Ninja',f'-DCMAKE_MAKE_PROGRAM={ninja}',
         f'-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake','-DANDROID_ABI='+args.abi,
         '-DANDROID_PLATFORM=android-21',f'-DSDL_SOURCE_DIR={sdl}','-DCMAKE_BUILD_TYPE=Release',
         '-DREGIONS='+';'.join(args.regions)])
    run([ninja,'-C',native,'-j',args.jobs])
    for region in args.regions:
        verify_fixed_symbols(native/f'librekhcom_{region}.so')
    if args.native_only:return
    stage=ROOT/('build/android-app'+suffix);stage.mkdir(exist_ok=True)
    shutil.copytree(ROOT/'port/android/app',stage/'app',dirs_exist_ok=True)
    shutil.copytree(skeleton/'app/src/main/java/org',stage/'app/src/main/java/org',dirs_exist_ok=True)
    shutil.copytree(skeleton/'gradle',stage/'gradle',dirs_exist_ok=True)
    shutil.copy2(skeleton/'gradlew.bat',stage/'gradlew.bat')
    # Pin and verify the distribution Gradle's official wrapper downloads.
    sha=urllib.request.urlopen('https://services.gradle.org/distributions/gradle-8.12-bin.zip.sha256',timeout=60).read().decode().strip()
    (stage/'gradle/wrapper/gradle-wrapper.properties').write_text(
        'distributionBase=GRADLE_USER_HOME\ndistributionPath=wrapper/dists\n'
        'distributionUrl=https\\://services.gradle.org/distributions/gradle-8.12-bin.zip\n'
        f'distributionSha256Sum={sha}\nzipStoreBase=GRADLE_USER_HOME\nzipStorePath=wrapper/dists\n')
    (stage/'settings.gradle').write_text("pluginManagement { repositories { google(); mavenCentral(); gradlePluginPortal() } }\ndependencyResolutionManagement { repositories { google(); mavenCentral() } }\nrootProject.name='ReKHCoM'\ninclude ':app'\n")
    (stage/'build.gradle').write_text("plugins { id 'com.android.application' version '8.7.3' apply false }\n")
    (stage/'gradle.properties').write_text('org.gradle.jvmargs=-Xmx1536m -Dfile.encoding=UTF-8\norg.gradle.workers.max=2\norg.gradle.daemon=false\n')
    (stage/'local.properties').write_text('sdk.dir='+str(sdk).replace('\\','/')+'\nndk.dir='+str(ndk).replace('\\','/')+'\n')
    (stage/'app/build.gradle').write_text("""plugins { id 'com.android.application' }
android {
 namespace 'com.rekhcom.android'
 compileSdk 35
 defaultConfig { applicationId 'com.rekhcom.android'; minSdk 21; targetSdk 35; versionCode 19001; versionName '1.9-android-alpha1' }
 compileOptions { sourceCompatibility JavaVersion.VERSION_1_8; targetCompatibility JavaVersion.VERSION_1_8 }
 buildTypes { debug { debuggable true }; release { minifyEnabled false } }
 lint { abortOnError true }
 packaging { jniLibs { useLegacyPackaging true } }
}
""")
    libs=stage/'app/src/main/jniLibs'/args.abi;libs.mkdir(parents=True,exist_ok=True)
    desired={'libSDL3.so',*(f'librekhcom_{r}.so' for r in args.regions)}
    # Remove only stale native binaries from this generated staging folder.
    for old in libs.glob('*.so'):
        assert old.resolve().is_relative_to(stage.resolve())
        if old.name not in desired:old.unlink()
    strip=ndk/'toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-strip.exe'
    for name in desired:
        source=native/('SDL/'+name if name=='libSDL3.so' else name)
        shutil.copy2(source,libs/name);run([strip,'--strip-debug',libs/name])
    notices=stage/'app/src/main/assets/licenses';notices.mkdir(parents=True,exist_ok=True)
    for source,name in [(sdl/'LICENSE.txt','SDL3.txt'),(ROOT/'LICENSE.md','ReKHCoM.md'),(ROOT/'port/android/assets/NOTICE.md','touch.md')]:shutil.copy2(source,notices/name)
    run([stage/'gradlew.bat','--no-daemon','--max-workers=2','assembleDebug'],cwd=stage)
    apk=stage/'app/build/outputs/apk/debug/app-debug.apk'
    import zipfile
    with zipfile.ZipFile(apk) as z:
        assert z.testzip() is None
        assert not any(Path(n).suffix.lower() in ['.gba','.sav','.khst','.ini'] for n in z.namelist())
        for name in desired:assert f'lib/{args.abi}/{name}' in z.namelist()
    out=ROOT/'build/releases';out.mkdir(exist_ok=True)
    target=out/f'ReKHCoM-v1.9-android-alpha1-{args.abi}-debug.apk'
    shutil.copy2(apk,target)
    digest=hashlib.sha256(target.read_bytes()).hexdigest()
    target.with_suffix('.apk.sha256').write_text(f'{digest}  {target.name}\n')
    (out/('android-build-manifest'+suffix+'.json')).write_text(json.dumps({'apk':target.name,'sha256':digest,'regions':args.regions,
        'abi':args.abi,'version':'1.9-android-alpha1','debug_signed':True,'runtime_tested':False},indent=2))
    print('Built (runtime verification still required):',target,digest)


if __name__=='__main__':main()
