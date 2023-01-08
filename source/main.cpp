#include "config.h"
#include "globals.h"
#include "patches.h"
#include "utils/logger.h"
#include <cstring>
#include <function_patcher/function_patching.h>
#include <vector>
#include <wups.h>

WUPS_PLUGIN_NAME("rce_patches");
WUPS_PLUGIN_DESCRIPTION("Patches security issues in WiiU games that could be triggered remotely");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Rambo6Glaz, Maschell");
WUPS_PLUGIN_LICENSE("");
WUPS_USE_STORAGE("rce_patches"); // Unique id for the storage api

std::vector<PatchData> mk8Patches;
std::vector<PatchData> splatoonPatches;

void RemovePatches(std::vector<PatchData> &patchHandles) {
    for (auto &patch : patchHandles) {
        if (FunctionPatcher_RemoveFunctionPatch(patch.handle) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to remove function patch %08X", patch.handle);
        }
    }
    patchHandles.clear();
}

INITIALIZE_PLUGIN() {
    DEBUG_FUNCTION_LINE("Patch functions");
    if (FunctionPatcher_InitLibrary() != FUNCTION_PATCHER_RESULT_SUCCESS) {
        OSFatal("rce_patches: FunctionPatcher_InitLibrary failed");
    }
    readStorage();
}

DEINITIALIZE_PLUGIN() {
    RemovePatches(mk8Patches);
    RemovePatches(splatoonPatches);
}

ON_APPLICATION_START() {
    initLogging();

    if (!gActivateMK8Patches && !mk8Patches.empty()) {
        DEBUG_FUNCTION_LINE("Remove MK8 patches");
        RemovePatches(mk8Patches);
    } else if (gActivateMK8Patches && mk8Patches.empty()) {
        DEBUG_FUNCTION_LINE("Add MK8 patches");
        if (!MARIO_KART_8_AddPatches(mk8Patches)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to add Mario Kart patches");
            RemovePatches(mk8Patches);
        }
    }

    if (!gActivateSplatoonPatches && !splatoonPatches.empty()) {
        DEBUG_FUNCTION_LINE("Remove Splatoon patches");
        RemovePatches(splatoonPatches);
    } else if (gActivateSplatoonPatches && splatoonPatches.empty()) {
        DEBUG_FUNCTION_LINE("Add Splatoon patches");
        if (!SPLATOON_AddPatches(splatoonPatches)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to add Splatoon patches");
            RemovePatches(splatoonPatches);
        }
    }

    for (auto &patch : mk8Patches) {
        bool isPatched = false;
        if (FunctionPatcher_IsFunctionPatched(patch.handle, &isPatched) == FUNCTION_PATCHER_RESULT_SUCCESS && isPatched) {
            DEBUG_FUNCTION_LINE("MK8: Function %s (handle: %08X) has been patched", patch.name.c_str(), patch.handle);
        } else {
            DEBUG_FUNCTION_LINE_ERR("Splatoon: Function %s (handle: %08X) has NOT been patched", patch.name.c_str(), patch.handle);
        }
    }
    for (auto &patch : splatoonPatches) {
        bool isPatched = false;
        if (FunctionPatcher_IsFunctionPatched(patch.handle, &isPatched) == FUNCTION_PATCHER_RESULT_SUCCESS && isPatched) {
            DEBUG_FUNCTION_LINE("Splatoon: Function %s (handle: %08X) has been patched", patch.name.c_str(), patch.handle);
        } else {
            DEBUG_FUNCTION_LINE_ERR("Splatoon: Function %s (handle: %08X) has NOT been patched", patch.name.c_str(), patch.handle);
        }
    }
}

ON_APPLICATION_ENDS() {
    deinitLogging();
}