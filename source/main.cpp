#include <wups.h>
#include <string.h>

#include "patches.h"

WUPS_PLUGIN_NAME("rce_patches");
WUPS_PLUGIN_DESCRIPTION("Patches security issues in WiiU games that could be triggered remotely");
WUPS_PLUGIN_VERSION("v2.0");
WUPS_PLUGIN_AUTHOR("GaryOderNichts");
WUPS_PLUGIN_LICENSE("MIT");

std::optional<rplinfo> gRPLInfo;

ON_APPLICATION_START()
{

	// If this is not a supported title, no need to do anything
	uint64_t titleId = OSGetTitleID();
	GamePatches *gamePatch = nullptr;
	for (auto &patch : sGamePatchList)
	{
		for (int i = 0; i < 3; i++)
		{
			if (patch.mRegionalTIDs[i] == titleId)
			{
				gamePatch = &patch;
				break;
			}
		}
	}

	if (!gamePatch)
		return;

	// Init logging
	if (!WHBLogModuleInit())
	{
		WHBLogCafeInit();
		WHBLogUdpInit();
	}

	WHBLogPrintf("rce_patches: applying patches for %s...", gamePatch->mTitleName);

	// Patch the dynload functions so GetRPLInfo works
	if (!PatchDynLoadFunctions())
	{
		WHBLogPrintf("rce_patches: Failed to patch dynload functions");
		return;
	}

	// Get the RPLInfo
	gRPLInfo = TryGetRPLInfo();
	if (!gRPLInfo)
	{
		WHBLogPrintf("rce_patches: Failed to get RPL info");
		return;
	}

	// For each patch type, call apply patch func (terrible design lol)
	for (auto &patch : gamePatch->mPatchTypes)
	{
		gamePatch->mPatchFunc(patch);
	}
}
