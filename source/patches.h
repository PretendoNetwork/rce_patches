#pragma once

#include <coreinit/cache.h>
#include <coreinit/memorymap.h>
#include <coreinit/title.h>
#include <cstdint>
#include <cstring>
#include <function_patcher/fpatching_defines.h>
#include <kernel/kernel.h>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct sead_String {
    char *mBuffer;
    uint32_t vtable;
};
WUT_CHECK_OFFSET(sead_String, 0x00, mBuffer);
WUT_CHECK_OFFSET(sead_String, 0x04, vtable);

struct __attribute__((__packed__)) enl_RecordHeader {
    uint8_t mContentTransporterID;
    uint16_t mContentLength;
};
WUT_CHECK_OFFSET(enl_RecordHeader, 0x00, mContentTransporterID);
WUT_CHECK_OFFSET(enl_RecordHeader, 0x01, mContentLength);

struct enl_Buffer {
    uint8_t *mData;
    size_t mCapacity;
    size_t mSize;
    bool isAllocated;
};
WUT_CHECK_OFFSET(enl_Buffer, 0x00, mData);
WUT_CHECK_OFFSET(enl_Buffer, 0x04, mCapacity);
WUT_CHECK_OFFSET(enl_Buffer, 0x08, mSize);
WUT_CHECK_OFFSET(enl_Buffer, 0x0C, isAllocated);

struct enl_ContentTransporter {
    struct ContentTransporterVtbl {
        int a;
        int b;
        int f08;
        void (*init)(enl_ContentTransporter *_this);
        int _f10;
        void *(*getSendBuffer)(enl_ContentTransporter *_this);
        int _f18;
        int (*getSendBufferSize)(enl_ContentTransporter *_this);
        int _f20;
        bool (*isNeedSend)(enl_ContentTransporter *_thiss);
        int _f28;
        unsigned char (*getContentID)(enl_ContentTransporter *_this);
        // ...
    };
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x00, a);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x04, b);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x08, f08);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x0C, init);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x10, _f10);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x14, getSendBuffer);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x18, _f18);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x1C, getSendBufferSize);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x20, _f20);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x24, isNeedSend);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x28, _f28);
    WUT_CHECK_OFFSET(ContentTransporterVtbl, 0x2C, getContentID);
    ContentTransporterVtbl *vtable;
};
WUT_CHECK_OFFSET(enl_ContentTransporter, 0x00, vtable);


class PatchData {
public:
    PatchData(std::string name, PatchedFunctionHandle handle) : name(std::move(name)), handle(handle) {
    }

    std::string name;
    PatchedFunctionHandle handle;
};

// ==========================================================================================

#define MARIO_KART_8_TID_J 0x000500001010EB00
#define MARIO_KART_8_TID_U 0x000500001010EC00
#define MARIO_KART_8_TID_E 0x000500001010ED00

#define MARIO_KART_8_TID   MARIO_KART_8_TID_J, MARIO_KART_8_TID_U, MARIO_KART_8_TID_E

bool MARIO_KART_8_AddPatches(std::vector<PatchData> &functionPatches);

// ==========================================================================================

#define SPLATOON_TID_J   0x0005000010162B00
#define SPLATOON_TID_U   0x0005000010176900
#define SPLATOON_TID_E   0x0005000010176A00

#define SPLATOON_TID     SPLATOON_TID_J, SPLATOON_TID_U, SPLATOON_TID_E
#define SPLATOON_PATCHES PATCH_ENL_BUFFER_RCE

bool SPLATOON_AddPatches(std::vector<PatchData> &functionPatches);

// ==========================================================================================
