#ifndef REKHCOM_SAVE_FILE_H
#define REKHCOM_SAVE_FILE_H
#include <windows.h>
#include <filesystem>
#include <vector>
#include <string>
namespace RekhcomSave {
/* Same-directory staging; no successful backup means caller must not call. */
inline bool Replace(const std::filesystem::path& target,const std::vector<unsigned char>& data) {
    std::filesystem::path temp;
    HANDLE file=INVALID_HANDLE_VALUE;
    static unsigned serial=0;
    for(int attempt=0;attempt<16&&file==INVALID_HANDLE_VALUE;attempt++) {
        temp=target;temp+=L".import-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(++serial);
        file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE&&GetLastError()!=ERROR_FILE_EXISTS)return false;
    }
    if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;
    bool ok=WriteFile(file,data.data(),static_cast<DWORD>(data.size()),&written,nullptr)
            &&written==data.size()&&FlushFileBuffers(file);
    if(!CloseHandle(file))ok=false;
    if(ok)ok=MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok)DeleteFileW(temp.c_str());
    return ok;
}
}
#endif
