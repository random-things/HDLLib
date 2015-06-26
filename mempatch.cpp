#include "mempatch.h"

DWORD	dwAddress;		// The address of the patch, stored global for protection.
DWORD	dwLength;		// The length of the patch.
char	*pSavedBuffer;	// A buffer to return the old contents to the host.

DWORD Protect(DWORD ProtectType)
{
	DWORD OldProtectType;
	VirtualProtect((LPVOID)dwAddress, dwLength, ProtectType, &OldProtectType);
	return OldProtectType;
}

BOOL PatchMemory(DWORD dwPatchAddress, void *pPatchBuffer, DWORD dwPatchLength)
{
	dwAddress = dwPatchAddress;
	dwLength = dwPatchLength;

	pSavedBuffer = new char[dwLength];
	if( !pSavedBuffer )
		return FALSE;

	DWORD oldMem = Protect(PAGE_EXECUTE_READWRITE);
	if( !oldMem ) // Protection failed.
		return FALSE;

	memcpy(pSavedBuffer, (LPVOID)dwAddress, dwLength);
	memcpy((LPVOID)dwAddress, (LPVOID)pPatchBuffer, dwLength);
	Protect(oldMem);

	return TRUE;
}