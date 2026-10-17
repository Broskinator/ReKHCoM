#ifndef REKHCOM_SAVE_IMPORT_H
#define REKHCOM_SAVE_IMPORT_H
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace RekhcomSave {
inline uint16_t Checksum(const unsigned char* p, size_t size) {
    uint32_t sum=0;
    for(size_t i=0;i+1<size;i+=2) sum+=p[i]|(p[i+1]<<8);
    if(size&1)sum+=p[size-1];
    sum=(sum&65535)+(sum>>16);
    return static_cast<uint16_t>(~(sum+(sum>>16)));
}
struct Block { size_t offset,size; };
inline constexpr Block blocks[]={
    {0x10,0x40},{0x50,0x40},{0x90,0xf14},{0xfa4,0xf14},
    {0x1ec0,0x418},{0x22d8,0x418},{0x26f0,0x418},{0x2b08,0x418},
    {0x2f20,0xf14},{0x3e34,0xf14},{0x4d48,0xf14},{0x5c5c,0xf14}
};
inline int Version(const unsigned char* p) {
    if(!std::memcmp(p,"KHCOM_BACKUP_VER00000013",24))return 13;
    if(!std::memcmp(p,"KHCOM_BACKUP_VER00000012",24))return 12;
    return 0;
}
/* Detect a complete SRAM image inside a raw save or emulator container.
 * A valid backup header is sufficient; never realign SRAM to that copy.
 * Distinct images in a multi-bank container are ambiguous and rejected. */
inline int Convert(const std::vector<unsigned char>& input,int region,
                   std::vector<unsigned char>& output,int& fromRegion) {
    output.clear();fromRegion=-1;
    if(region<0||region>2||input.size()<32768||input.size()>(4u<<20))return -1;
    std::vector<unsigned char> selected;
    int sourceVersion=0,goodCount=0;
    bool ambiguous=false;
    for(size_t at=0;at+0x40<=input.size();at++) {
        int version=Version(input.data()+at);
        if(!version||Checksum(input.data()+at,0x40)!=0)continue;
        for(size_t headerOffset:{size_t(0x10),size_t(0x50)}) {
            if(at<headerOffset)continue;
            size_t base=at-headerOffset;
            if(input.size()-base<32768)continue;
            int headers=0,files=0,valid=0;
            bool mixed=false;
            for(const auto& block:blocks) {
                const unsigned char* p=input.data()+base+block.offset;
                int v=Version(p);
                // A short header followed by erased SRAM can also satisfy a
                // longer ones-complement checksum. Check initialized common
                // data so it cannot masquerade as a system/file block.
                bool initialized=block.size==0x40 ||
                    (p[0xac]!=255 && p[0xad]!=255 &&
                     !(p[28]==255&&p[29]==255&&p[30]==255&&p[31]==255));
                if(v&&initialized&&Checksum(p,block.size)==0) {
                    if(v!=version)mixed=true;
                    valid++;
                    if(block.size==0x40)headers++;else files++;
                }
            }
            if(mixed||!headers||!files)continue;
            std::vector<unsigned char> candidate(65536,255);
            std::memcpy(candidate.data(),input.data()+base,std::min<size_t>(65536,input.size()-base));
            if(valid>goodCount) {
                selected=std::move(candidate);sourceVersion=version;goodCount=valid;ambiguous=false;
            } else if(valid==goodCount && selected!=candidate) {
                ambiguous=true;
            }
        }
    }
    if(selected.empty()||ambiguous)return -1;
    const char* signature=region==0?"KHCOM_BACKUP_VER00000013":"KHCOM_BACKUP_VER00000012";
    for(const auto& block:blocks) {
        auto* p=selected.data()+block.offset;
        if(Version(p)!=sourceVersion||Checksum(p,block.size)!=0)continue;
        std::memcpy(p,signature,24);
        // SaveHeader.data begins at 0x1c: flags at 0x1c, language at 0x1e.
        // Clearing 0x1c would silently erase Sora/Riku completion flags.
        if(block.size==0x40&&region!=0)p[0x1e]=p[0x1f]=0;
        p[24]=p[25]=0;
        uint16_t sum=Checksum(p,block.size);
        p[24]=static_cast<unsigned char>(sum);p[25]=static_cast<unsigned char>(sum>>8);
    }
    output=std::move(selected);fromRegion=sourceVersion==13?0:1;
    return goodCount;
}
}
#endif
