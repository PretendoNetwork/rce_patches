#include "patches.h"
#include "utils/logger.h"
#include <function_patcher/function_patching.h>
#include <set>

// ==========================================================================================
DECL_FUNCTION(bool, enl_ParseIdentificationToken, void *identifiationInfo, sead_String *identificationToken) {
    // Fix for RCE (stack overflow if identification buffer was bigger than 16)
    if (strnlen(identificationToken->mBuffer, 16) == 16) {
        DEBUG_FUNCTION_LINE_INFO("Avoided bufferoverlow in enl_ParseIdentificationToken!");
        identificationToken->mBuffer[15] = '\0';
        return real_enl_ParseIdentificationToken(identifiationInfo, identificationToken);
    }

    return real_enl_ParseIdentificationToken(identifiationInfo, identificationToken);
}

DECL_FUNCTION(enl_ContentTransporter *, enl_TransportManager_getContentTransporter, void *_this, unsigned char &id) {
    return real_enl_TransportManager_getContentTransporter(_this, id);
}

DECL_FUNCTION(void, enl_TransportManager_updateReceiveBuffer_, void *_this, signed char const &bufferId, uint8_t *data, uint32_t size) {

    static enl_RecordHeader s_TerminationRecord = {.mContentTransporterID = 0xff, .mContentLength = 0};

    // Check for end record in the data, if there is not, drop packet
    bool hasEndRecord = false;

    // Loop through all records and check if there's a bad record (size mismatch) until out of bounds or end record
    uint8_t *pData = data;
    while (pData < (data + size)) {
        enl_RecordHeader *record = (enl_RecordHeader *) pData;
        if (record->mContentLength == 0 && record->mContentTransporterID == 0xff) {
            hasEndRecord = true;
            break;
        }

        enl_ContentTransporter *contentTransp = real_enl_TransportManager_getContentTransporter(_this, record->mContentTransporterID);
        // Actual fix for the ENL nullptr deref crash (lmao)
        if (!contentTransp) {
            DEBUG_FUNCTION_LINE_INFO("Avoided ENL nullptr deref crash in enl_TransportManager_updateReceiveBuffer_!");
            return real_enl_TransportManager_updateReceiveBuffer_(_this, bufferId, (uint8_t *) &s_TerminationRecord, sizeof(enl_RecordHeader));
        }

        if (record->mContentLength > 0x440) {
            DEBUG_FUNCTION_LINE_INFO("record->mContentLength was over 0x440 in enl_TransportManager_updateReceiveBuffer_!");
            return real_enl_TransportManager_updateReceiveBuffer_(_this, bufferId, (uint8_t *) &s_TerminationRecord, sizeof(enl_RecordHeader));
        }

        pData += sizeof(enl_RecordHeader);
        pData += record->mContentLength;
    }

    if (!hasEndRecord) {
        return real_enl_TransportManager_updateReceiveBuffer_(_this, bufferId, (uint8_t *) &s_TerminationRecord, sizeof(enl_RecordHeader));
    }

    return real_enl_TransportManager_updateReceiveBuffer_(_this, bufferId, data, size);
}

DECL_FUNCTION(void, enl_Buffer_set, enl_Buffer *_this, uint8_t const *data, size_t size) {
    // Fix for the RCE
    if (!_this->mData || !size || size > _this->mCapacity) {
        DEBUG_FUNCTION_LINE_INFO("Avoided overflow in enl_Buffer_set!");
        return;
    }

    memcpy(_this->mData, data, size);
    _this->mSize = size;
}
// ==========================================================================================

bool MARIO_KART_8_AddPatches(std::vector<PatchData> &functionPatches) {
    uint64_t titleIds[]              = {MARIO_KART_8_TID};
    function_replacement_data_t repl = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_ParseIdentificationToken,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Turbo.rpx",
            0x8E3930, // Address of 'enl::PiaUtil::ParseIdentificationToken'
            64, 64);
    PatchedFunctionHandle handle = 0;
    if (FunctionPatcher_AddFunctionPatch(&repl, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_ParseIdentificationToken\" patch");
        return false;
    }
    functionPatches.emplace_back("enl::PiaUtil::ParseIdentificationToken", handle);

    function_replacement_data_t repl1 = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_TransportManager_getContentTransporter,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Turbo.rpx",
            0x8D7678, // Address of 'enl::TransportManager::getContentTransporter'
            64, 64);
    if (FunctionPatcher_AddFunctionPatch(&repl1, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_TransportManager_getContentTransporter\" patch");
        return false;
    }
    functionPatches.emplace_back("enl::TransportManager::getContentTransporter", handle);


    function_replacement_data_t repl2 = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_TransportManager_updateReceiveBuffer_,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Turbo.rpx",
            0x8D772C, // Address of 'enl::TransportManager::updateReceiveBuffer_'
            64, 64);
    if (FunctionPatcher_AddFunctionPatch(&repl2, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_TransportManager_updateReceiveBuffer_\" patch");
        return false;
    }
    functionPatches.emplace_back("enl::TransportManager::updateReceiveBuffer_", handle);

    function_replacement_data_t repl3 = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_Buffer_set,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Turbo.rpx",
            0x8CF228,
            64, 64);
    if (FunctionPatcher_AddFunctionPatch(&repl3, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_Buffer_set\" patch");
        return false;
    }
    functionPatches.emplace_back("enl:Buffer::set", handle);

    return true;
}

// ==========================================================================================

bool SPLATOON_AddPatches(std::vector<PatchData> &functionPatches) {
    uint64_t titleIds[]              = {SPLATOON_TID};
    function_replacement_data_t repl = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_TransportManager_getContentTransporter,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Gambit.rpx",
            0xB4108C, // Address of 'enl::TransportManager::getContentTransporter'
            272, 272);
    PatchedFunctionHandle handle;
    if (FunctionPatcher_AddFunctionPatch(&repl, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_TransportManager_getContentTransporter\" patch");
        return false;
    }
    functionPatches.emplace_back("enl::TransportManager::getContentTransporter", handle);

    function_replacement_data_t repl1 = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_TransportManager_updateReceiveBuffer_,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Gambit.rpx",
            0xB41140, // Address of 'enl::TransportManager::updateReceiveBuffer_'
            272, 272);
    if (FunctionPatcher_AddFunctionPatch(&repl1, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_TransportManager_updateReceiveBuffer_\" patch");
        return false;
    }
    functionPatches.emplace_back("enl::TransportManager::updateReceiveBuffer_", handle);

    function_replacement_data_t repl2 = REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
            enl_Buffer_set,
            titleIds, sizeof(titleIds) / sizeof(titleIds[0]),
            "Gambit.rpx",
            0xB4D178, // Address of 'enl:Buffer::set'
            272, 272);
    if (FunctionPatcher_AddFunctionPatch(&repl2, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add \"enl_Buffer_set\" patch");
        return false;
    }
    functionPatches.emplace_back("enl:Buffer::set", handle);
    return true;
}
