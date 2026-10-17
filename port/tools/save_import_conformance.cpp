#include "save_import.h"
#include "save_file.h"
#include <cassert>
#include <fstream>
#include <iostream>
using Bytes=std::vector<unsigned char>;
static int checks;
static void Check(bool condition){if(!condition)throw std::runtime_error("save import check failed #"+std::to_string(checks+1));checks++;}
static void Block(Bytes& data,size_t at,size_t size,int version) {
    std::memcpy(data.data()+at,version==13?"KHCOM_BACKUP_VER00000013":"KHCOM_BACKUP_VER00000012",24);
    data[at+24]=data[at+25]=0;
    auto sum=RekhcomSave::Checksum(data.data()+at,size);
    data[at+24]=sum;data[at+25]=sum>>8;
}
static Bytes Save(int version) {
    Bytes data(65536,255);
    for(size_t at:{size_t(0x10),size_t(0x50)}) {
        data[at+0x1c]=7;data[at+0x1d]=0; // completion flags
        data[at+0x1e]=4;data[at+0x1f]=0; // EU Spanish
        Block(data,at,0x40,version);
    }
    data[0x2f20+28]=0;data[0x2f20+0xac]=1;data[0x2f20+0xad]=0;
    Block(data,0x2f20,0xf14,version);
    return data;
}
static Bytes Read(const std::filesystem::path& path) {
    std::ifstream in(path,std::ios::binary);
    return Bytes(std::istreambuf_iterator<char>(in),{});
}
int main() {
    Bytes output;int from;
    for(int version:{12,13})for(int region=0;region<3;region++) {
        Bytes input=Save(version),original=input;
        Check(RekhcomSave::Convert(input,region,output,from)==3);
        Check(input==original);
        Check(from==(version==13?0:1));
        for(size_t at:{size_t(0x10),size_t(0x50)}) {
            Check(output[at+0x1c]==7&&output[at+0x1d]==0);
            Check(output[at+0x1e]==(region==0?4:0));
            Check(RekhcomSave::Checksum(output.data()+at,0x40)==0);
            Check(RekhcomSave::Version(output.data()+at)==(region==0?13:12));
        }
        Check(RekhcomSave::Checksum(output.data()+0x2f20,0xf14)==0);
        if(version==13&&region==0)Check(output==input);
    }
    Bytes input=Save(13),bad=input;
    bad[0x10+28]^=1;
    Check(RekhcomSave::Convert(bad,0,output,from)==2);
    Check(output==bad); // primary corruption must not shift all other slots
    bad[0x50+28]^=1;Check(RekhcomSave::Convert(bad,0,output,from)<0&&output.empty());
    bad=input;bad[0x2f20+28]^=1;Check(RekhcomSave::Convert(bad,0,output,from)<0);
    Bytes wrapped(512,0x33);wrapped.insert(wrapped.end(),input.begin(),input.end());
    wrapped.insert(wrapped.end(),128,0xaa);
    int wrappedResult=RekhcomSave::Convert(wrapped,0,output,from);
    if(wrappedResult!=3||output!=input)std::cerr<<"wrapped result "<<wrappedResult<<" size "<<output.size()<<" equal "<<(output==input)<<"\n";
    Check(wrappedResult==3&&output==input);
    Bytes small(input.begin(),input.begin()+32768);
    Check(RekhcomSave::Convert(small,0,output,from)==3&&output==input);
    Bytes twice=input;twice.insert(twice.end(),input.begin(),input.end());
    Check(RekhcomSave::Convert(twice,0,output,from)==3&&output==input);
    twice[65536+10000]^=1;Check(RekhcomSave::Convert(twice,0,output,from)<0);
    Check(RekhcomSave::Convert(Bytes(65536,0),0,output,from)<0);
    Check(RekhcomSave::Convert(input,3,output,from)<0);
    Check(RekhcomSave::Convert(Bytes(32767,255),0,output,from)<0);
    Check(RekhcomSave::Convert(Bytes((4u<<20)+1,255),0,output,from)<0);
    // Actual Windows staging / replacement, including a locked destination.
    namespace fs=std::filesystem;
    fs::path directory=fs::current_path()/std::to_string(GetCurrentProcessId());
    Check(!fs::exists(directory)&&fs::create_directory(directory));
    fs::path target=directory/L"partida.sav";
    Check(RekhcomSave::Replace(target,input)&&Read(target)==input);
    Bytes replacement=Save(12);
    HANDLE locked=CreateFileW(target.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
    Check(locked!=INVALID_HANDLE_VALUE);
    Check(!RekhcomSave::Replace(target,replacement));
    Check(Read(target)==input);
    CloseHandle(locked);
    Check(RekhcomSave::Replace(target,replacement)&&Read(target)==replacement);
    Check(!RekhcomSave::Replace(directory/L"missing"/L"partida.sav",input));
    int files=0;for(const auto& entry:fs::directory_iterator(directory)){Check(entry.path()==target);files++;}
    Check(files==1);Check(fs::remove(target));Check(fs::remove(directory));
    std::cout<<"Save import / atomic replacement: "<<checks<<" checks PASS\n";
}
