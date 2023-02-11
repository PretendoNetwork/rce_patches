#include "config.h"
#include "globals.h"
#include "patches.h"
#include "utils/logger.h"
#include <coreinit/title.h>
#include <string>
#include <sysapp/launch.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#define MK8PATCHES_CONFIG_ID      "mk8patches"
#define SPLATOONPATCHES_CONFIG_ID "splatoonpatches"

#define LOAD_BOOL_FROM_CONFIG(config_name, __variable__)                                                          \
    if ((storageRes = WUPS_GetBool(nullptr, config_name, &__variable__)) == WUPS_STORAGE_ERROR_NOT_FOUND) {       \
        if (WUPS_StoreBool(nullptr, config_name, __variable__) != WUPS_STORAGE_ERROR_SUCCESS) {                   \
            DEBUG_FUNCTION_LINE_WARN("Failed to store bool");                                                     \
        }                                                                                                         \
    } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {                                                        \
        DEBUG_FUNCTION_LINE_WARN("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes); \
    }                                                                                                             \
    while (0)


#define PROCESS_BOOL_ITEM_CHANGED(__config__name, __variable__)                       \
    if (std::string_view(item->configId) == __config__name) {                         \
        DEBUG_FUNCTION_LINE_VERBOSE("New value in %s: %d", __config__name, newValue); \
        __variable__ = newValue;                                                      \
        WUPS_StoreInt(nullptr, __config__name, __variable__);                         \
        return;                                                                       \
    }                                                                                 \
    while (0)

bool prevActivateMK8Patches      = true;
bool prevActivateSplatoonPatches = true;

void readStorage() {
    // Open storage to read values
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
    } else {
        LOAD_BOOL_FROM_CONFIG(MK8PATCHES_CONFIG_ID, gActivateMK8Patches);
        LOAD_BOOL_FROM_CONFIG(SPLATOONPATCHES_CONFIG_ID, gActivateSplatoonPatches);

        prevActivateMK8Patches      = gActivateMK8Patches;
        prevActivateSplatoonPatches = gActivateSplatoonPatches;

        // Close storage
        if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE("Failed to close storage");
        }
    }
}

void boolItemChanged(ConfigItemBoolean *item, bool newValue) {
    PROCESS_BOOL_ITEM_CHANGED(MK8PATCHES_CONFIG_ID, gActivateMK8Patches);
    PROCESS_BOOL_ITEM_CHANGED(SPLATOONPATCHES_CONFIG_ID, gActivateSplatoonPatches);
}

WUPS_GET_CONFIG() {
    // We open the storage, so we can persist the configuration the user did.
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to open storage");
        return 0;
    }

    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "RCE Patches");

    WUPSConfigCategoryHandle cat;
    WUPSConfig_AddCategoryByNameHandled(config, "Games", &cat);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, cat, MK8PATCHES_CONFIG_ID, "Patch exploits in Mario Kart 8", gActivateMK8Patches, &boolItemChanged);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, cat, SPLATOONPATCHES_CONFIG_ID, "Patch exploits in Splatoon", gActivateSplatoonPatches, &boolItemChanged);

    return config;
}

WUPS_CONFIG_CLOSED() {
    // Save all changes
    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to close storage");
    }

    if (prevActivateMK8Patches != gActivateMK8Patches) {
        if (OSGetTitleID() == MARIO_KART_8_TID_J ||
            OSGetTitleID() == MARIO_KART_8_TID_E ||
            OSGetTitleID() == MARIO_KART_8_TID_U) {
            _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
        }
    }

    if (prevActivateSplatoonPatches != gActivateSplatoonPatches) {
        if (OSGetTitleID() == SPLATOON_TID_J ||
            OSGetTitleID() == SPLATOON_TID_E ||
            OSGetTitleID() == SPLATOON_TID_U) {
            _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
        }
    }

    prevActivateMK8Patches      = gActivateMK8Patches;
    prevActivateSplatoonPatches = gActivateSplatoonPatches;
}
