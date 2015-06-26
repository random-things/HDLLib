#include <windows.h>
#include "AddressArray.h"

//
// This programs functions.
//
AddressArray *GetAddressArray();
DWORD SearchMemory(DWORD dwSearchType, DWORD dwSearchLength, LPVOID pSearchValue);
DWORD SearchNext(DWORD dwSearchType, DWORD dwSearchLength, LPVOID pSearchValue);
void HexDumpMemory(char *,char *,int);
LPWORD PreparePattern(char *String);
BOOL LocateFingerPrint(char *FingerPrint, unsigned long PatternLen, DWORD BeginningOfSearch, DWORD EndOfSearch);