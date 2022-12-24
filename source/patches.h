#pragma once

#include <cstdint>
#include <vector>
#include <cstring>

#include <whb/log.h>
#include <whb/log_module.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>

#include <coreinit/title.h>

#include <patcher/rplinfo.h>
#include <patcher/patcher.h>

#include <kernel/kernel.h>

#include <coreinit/cache.h>
#include <coreinit/memorymap.h>

#undef DECL_FUNCTION
#include <function_patcher/function_patching.h>

enum EPatchType
{
	PATCH_ENL_BUFFER_RCE,
	PATCH_ENL_ID_TOKEN_RCE,
};

struct GamePatches
{
	const char *mTitleName;
	uint64_t mRegionalTIDs[3];
	std::vector<EPatchType> mPatchTypes;
	void (*mPatchFunc)(EPatchType type);
};

// ==========================================================================================

struct sead_String
{
	char *mBuffer;
	uint32_t vtable;
};

struct __attribute__((__packed__)) enl_RecordHeader
{
	uint8_t mContentTransporterID;
	uint16_t mContentLength;
};

struct enl_ContentTransporter
{
	struct ContentTransporterVtbl
	{
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
	ContentTransporterVtbl *vtable;
};

#define MARIO_KART_8_TID_J 0x000500001010EB00
#define MARIO_KART_8_TID_U 0x000500001010EC00
#define MARIO_KART_8_TID_E 0x000500001010ED00

#define MARIO_KART_8_TID MARIO_KART_8_TID_J, MARIO_KART_8_TID_U, MARIO_KART_8_TID_E
#define MARIO_KART_8_PATCHES PATCH_ENL_BUFFER_RCE, PATCH_ENL_ID_TOKEN_RCE

void MARIO_KART_8_ApplyPatch(EPatchType type);

// ==========================================================================================

#define SPLATOON_TID_J 0x0005000010162B00
#define SPLATOON_TID_U 0x0005000010176900
#define SPLATOON_TID_E 0x0005000010176A00

#define SPLATOON_TID SPLATOON_TID_J, SPLATOON_TID_U, SPLATOON_TID_E
#define SPLATOON_PATCHES PATCH_ENL_BUFFER_RCE

void SPLATOON_ApplyPatch(EPatchType type);

// ==========================================================================================

extern std::optional<rplinfo> gRPLInfo;
static std::vector<GamePatches> sGamePatchList = {
	{"Mario Kart 8 ", {MARIO_KART_8_TID}, {MARIO_KART_8_PATCHES}, MARIO_KART_8_ApplyPatch},
	{"Splatoon", {SPLATOON_TID}, {SPLATOON_PATCHES}, SPLATOON_ApplyPatch},
};