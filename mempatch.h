#ifndef __MEMPATCH_H_
#define __MEMPATCH_H_

#include <windows.h>

BOOL PatchMemory(DWORD dwPatchAddress, void *pPatchBuffer, DWORD dwPatchLength);
DWORD Protect(DWORD ProtectType);

#endif