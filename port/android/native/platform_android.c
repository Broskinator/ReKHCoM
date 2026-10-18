/* Android alpha platform glue. Normal cartridge saves are fully supported;
 * pointer-based quick states and GDI HD text are unavailable in this backend. */
#include <SDL3/SDL.h>
#include <jni.h>
#include <stdint.h>
#include "port/port.h"
#include "port/port_state.h"
#include "port/port_ui.h"

extern char __start_khdata[], __stop_khdata[], __start_khbss[], __stop_khbss[];
void PortGameSections(char** data,size_t* size,char** bss,size_t* bsize) {
    *data=__start_khdata; *size=(size_t)(__stop_khdata-__start_khdata);
    *bss=__start_khbss; *bsize=(size_t)(__stop_khbss-__start_khbss);
}
int gPortInMainLoop, gPortStateSlot=1;
const char* PortStatePath(int slot) { (void)slot; return ""; }
void PortStateRequestSave(const char* path) { (void)path; PortUiNotice("Quick states unavailable in Android alpha"); }
void PortStateRequestLoad(const char* path) { (void)path; PortUiNotice("Quick states unavailable in Android alpha"); }
void PortStateService(void) {}
/* no states here: boss rematches stay hidden (port_ui.c) */
void PortStateRequestQuiet(int load, const char* path) { (void)load; (void)path; }
int PortStateBusy(void) { return 0; }
int PortStateLastResult(void) { return 0; }
void PortWatchdogStart(void) {}
void PortCrashHandlerInstall(void) {}
void PortBiosFaultInstall(void) { PortLog("Android: explicit BIOS_PTR helpers; no x86 fault recovery"); }
int __aeabi_idiv0(int value) { (void)value; return 0; }
long long __aeabi_ldiv0(long long value) { (void)value; return 0; }
unsigned PortSjisToCodepoint(unsigned code) {
    JNIEnv* env=(JNIEnv*)SDL_GetAndroidJNIEnv();
    jclass type; jmethodID method; jbyteArray bytes; jstring charset,text;
    jbyte input[2]={(jbyte)(code>>8),(jbyte)code}; jchar output;unsigned result=0;
    if(!env)return 0;
    bytes=(*env)->NewByteArray(env,2);(*env)->SetByteArrayRegion(env,bytes,0,2,input);
    charset=(*env)->NewStringUTF(env,"Shift_JIS");type=(*env)->FindClass(env,"java/lang/String");
    method=(*env)->GetMethodID(env,type,"<init>","([BLjava/lang/String;)V");
    text=(jstring)(*env)->NewObject(env,type,method,bytes,charset);
    if(!(*env)->ExceptionCheck(env) && text && (*env)->GetStringLength(env,text)==1) {
        (*env)->GetStringRegion(env,text,0,1,&output);result=output;
    } else (*env)->ExceptionClear(env);
    if(text)(*env)->DeleteLocalRef(env,text);
    (*env)->DeleteLocalRef(env,type);(*env)->DeleteLocalRef(env,charset);(*env)->DeleteLocalRef(env,bytes);
    return result;
}
int PortCodepointToSjis(unsigned cp) {
    JNIEnv* env;
    jclass type;
    jmethodID method;
    jstring text, charset;
    jbyteArray bytes;
    jbyte data[2];
    jchar code[2];
    int n, result=0;
    if(cp<128) return (int)cp;
    env=(JNIEnv*)SDL_GetAndroidJNIEnv();
    if(!env) return 0;
    if(cp>0xFFFF) { cp-=0x10000; code[0]=0xD800+(cp>>10); code[1]=0xDC00+(cp&1023); n=2; }
    else { code[0]=(jchar)cp; n=1; }
    text=(*env)->NewString(env,code,n);
    charset=(*env)->NewStringUTF(env,"Shift_JIS");
    type=(*env)->FindClass(env,"java/lang/String");
    method=(*env)->GetMethodID(env,type,"getBytes","(Ljava/lang/String;)[B");
    bytes=(jbyteArray)(*env)->CallObjectMethod(env,text,method,charset);
    if(!(*env)->ExceptionCheck(env) && bytes) {
        n=(*env)->GetArrayLength(env,bytes);
        if(n==1 || n==2) {
            (*env)->GetByteArrayRegion(env,bytes,0,n,data);
            result=n==2 ? ((unsigned char)data[0]<<8)|(unsigned char)data[1] : (unsigned char)data[0];
        }
    } else (*env)->ExceptionClear(env);
    if(bytes) (*env)->DeleteLocalRef(env,bytes);
    (*env)->DeleteLocalRef(env,type); (*env)->DeleteLocalRef(env,text); (*env)->DeleteLocalRef(env,charset);
    return result;
}
