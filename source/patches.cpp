#include "patches.h"

// ==========================================================================================

DECL_FUNCTION(bool, enl_ParseIdentificationToken, void *identifiationInfo, sead_String *identificationToken)
{
	// Fix for RCE (stack overflow if identification buffer was bigger than 16)
	if (strnlen(identificationToken->mBuffer, 16) == 16)
	{
		identificationToken->mBuffer[15] = '\0';
		return real_enl_ParseIdentificationToken(identifiationInfo, identificationToken);
	}

	return real_enl_ParseIdentificationToken(identifiationInfo, identificationToken);
}

enl_ContentTransporter *(*real_enl_TransportManager_getContentTransporter)(void *_this, unsigned char &id);
DECL_FUNCTION(void, enl_TransportManager_updateReceiveBuffer_, void *_this, signed char const &bufferId, uint8_t *data, uint32_t size)
{

	// Loop through all records and check if there's a bad record (size mismatch) until out of bounds or end record
	uint8_t *pData = data;
	while (pData < (data + size))
	{
		enl_RecordHeader *record = (enl_RecordHeader *)pData;
		if (record->mContentLength == 0 && record->mContentTransporterID == 0xff)
			break;

		enl_ContentTransporter *contentTransp = real_enl_TransportManager_getContentTransporter(_this, record->mContentTransporterID);
		// Actual fix for the ENL nullptr deref crash (lmao)
		if (!contentTransp)
			return;

		// Fix for RCE (if size mismatch, do not handle packet.)
		if (record->mContentLength > contentTransp->vtable->getSendBufferSize(contentTransp))
			return;

		pData += sizeof(enl_RecordHeader);
		pData += record->mContentLength;
	}

	return real_enl_TransportManager_updateReceiveBuffer_(_this, bufferId, data, size);
}

// ==========================================================================================

void MARIO_KART_8_ApplyPatch(EPatchType type)
{
	auto turbo_rpx = FindRPL(*gRPLInfo, "Turbo.rpx");
	if (!turbo_rpx)
	{
		WHBLogPrintf("rce_patches: Couldn't find Turbo.rpx ...");
		return;
	}

	if (type == PATCH_ENL_ID_TOKEN_RCE)
	{
		// Address of 'enl::PiaUtil::ParseIdentificationToken'
		uint32_t addr_func = turbo_rpx->textAddr + 0x8E3930;
		function_replacement_data_t repl = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
			enl_ParseIdentificationToken,
			OSEffectiveToPhysical(addr_func),
			addr_func,
			FP_TARGET_PROCESS_GAME_AND_MENU);
		FunctionPatcherPatchFunction(&repl, nullptr);

		WHBLogPrintf("rce_patches: Patched Mario Kart 8 (PATCH_ENL_ID_TOKEN_RCE)");
	}

	if (type == PATCH_ENL_BUFFER_RCE)
	{
		real_enl_TransportManager_getContentTransporter = (enl_ContentTransporter * (*)(void *, unsigned char &))(turbo_rpx->textAddr + 0x8D7678);

		// Address of 'enl::TransportManager::updateReceiveBuffer_'
		uint32_t addr_func = turbo_rpx->textAddr + 0x8D772C;
		function_replacement_data_t repl = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
			enl_TransportManager_updateReceiveBuffer_,
			OSEffectiveToPhysical(addr_func),
			addr_func,
			FP_TARGET_PROCESS_GAME_AND_MENU);
		FunctionPatcherPatchFunction(&repl, nullptr);

		WHBLogPrintf("rce_patches: Patched Mario Kart 8 (PATCH_ENL_BUFFER_RCE)");
	}
}

// ==========================================================================================

void SPLATOON_ApplyPatch(EPatchType type)
{

	auto gambit_rpx = FindRPL(*gRPLInfo, "Gambit.rpx");
	if (!gambit_rpx)
	{
		WHBLogPrintf("rce_patches: Couldn't find Gambit.rpx ...");
		return;
	}

	if (type == PATCH_ENL_BUFFER_RCE)
	{
		real_enl_TransportManager_getContentTransporter = (enl_ContentTransporter * (*)(void *, unsigned char &))(gambit_rpx->textAddr + 0xB4108C);

		// Address of 'enl::TransportManager::updateReceiveBuffer_'
		uint32_t addr_func = gambit_rpx->textAddr + 0xB41140;
		function_replacement_data_t repl = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
			enl_TransportManager_updateReceiveBuffer_,
			OSEffectiveToPhysical(addr_func),
			addr_func,
			FP_TARGET_PROCESS_GAME_AND_MENU);
		FunctionPatcherPatchFunction(&repl, nullptr);

		WHBLogPrintf("rce_patches: Patched Splatoon (PATCH_ENL_BUFFER_RCE)");
	}
}