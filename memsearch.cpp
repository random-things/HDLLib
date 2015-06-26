#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "AddressArray.h"
#include "memorysearch.h"
#include "ProcessMemory.h"

#include <psapi.h>
#pragma comment( lib, "psapi.lib" )

#define MAX_PROCESSES 100
#define MAX_MODULES 100
#define MAX_BASENAME 64

#define SEARCH_TYPE_BYTE		0x00000001
#define SEARCH_TYPE_WORD		0x00000002
#define SEARCH_TYPE_DWORD		0x00000003
#define SEARCH_TYPE_DOUBLE		0x00000004
#define	SEARCH_TYPE_STRING		0x00000005
#define SEARCH_TYPE_FINGERPRINT	0x00000006

struct ImageInfo {
	char ImageName[MAX_BASENAME];
	MODULEINFO ImageInformation;
} IMAGE;

AddressArray *myArray;

AddressArray *GetAddressArray()
{
	return myArray;
}

DWORD SearchNext(DWORD dwSearchType, DWORD dwSearchLength, LPVOID pSearchValue)
{
	if (!myArray )
		return -1;

	AddressArray *nextArray;
	nextArray = new AddressArray();

	char tempBuf[1024];

	unsigned int j;
	switch( dwSearchType)
	{
		// We can search the page!!!
		case SEARCH_TYPE_BYTE:
			for( j = 0; j < myArray->GetResultCount(); j++ )
			{
				if( !memcmp((unsigned char *)myArray->Index(j), pSearchValue, dwSearchLength ) ) // Returns zero if equal
				{
					nextArray->AppendAddress(myArray->Index(j));
				}
			}
			break;
		case SEARCH_TYPE_WORD:
			for( j = 0; j < myArray->GetResultCount(); j++ )
			{
				if( !memcmp((unsigned char *)myArray->Index(j), pSearchValue, dwSearchLength ) ) // Returns zero if equal
				{
					nextArray->AppendAddress(myArray->Index(j));
				}
			}
			break;
		case SEARCH_TYPE_DWORD:
			for( j = 0; j < myArray->GetResultCount(); j++ )
			{
				if( !memcmp((unsigned char *)myArray->Index(j), pSearchValue, dwSearchLength ) ) // Returns zero if equal
				{
					//sprintf(tempBuf, "Found a matching address at 0x%08x", myArray->Index(j));
					//MessageBox(0, tempBuf, NULL, MB_OK);
					nextArray->AppendAddress(myArray->Index(j));
				}
			}
			break;
		/*case SEARCH_TYPE_DOUBLE:
			break;*/
		case SEARCH_TYPE_STRING:
			for( j = 0; j < myArray->GetResultCount(); j++ )
			{
				if (!strncmp((char *)myArray->Index(j),(const char *)pSearchValue,strlen((const char *)pSearchValue)))
				{
					//HexDumpMemory((char *)(PhysicalAddress+i),(char *)(PhysicalAddress+i),(MemoryLength-i<256)?MemoryLength-i:256);
					nextArray->AppendAddress(myArray->Index(j));
				}
			}
			break;
		case SEARCH_TYPE_FINGERPRINT:
			MessageBox(0, "SEARCH_TYPE_FINGERPRINT: Invalid type specified for SearchNext()\n\nFingerprints may only be searched for once.", NULL, MB_OK);
			break;
	}

	delete myArray;

	myArray = new AddressArray();
	for( unsigned int i=0; i<nextArray->GetResultCount(); i++)
	{
		//sprintf(tempBuf, "Address: 0x%08x", nextArray->Index(i));
		//MessageBox(0, tempBuf, "Copying over...", MB_OK);
		myArray->AppendAddress(nextArray->Index(i));
	}

	delete nextArray;

	return myArray->GetResultCount();
}

DWORD SearchMemory(DWORD dwSearchType, DWORD dwSearchLength, LPVOID in_pSearchValue)
//
// Function that searches all of physical memory for the given string.
//
{
	char pSearchValue[1024];
	memcpy(pSearchValue, in_pSearchValue, dwSearchLength);
	memset(in_pSearchValue, 0, dwSearchLength);

	CMemoryWalker myWalker;

   // Make sure we free the previous array.
   if( myArray )
	   delete myArray;
   myArray = new AddressArray();


   LPVMOBJECT pPage;
   int numPages;
   myWalker.memWalk();
   pPage = myWalker.mGetPageList(&numPages);
   if( numPages == 0 )
	   return -1;

   char tempBuf[1024];
   //sprintf(tempBuf, "The memory walker found %d pages in the process.", numPages);
   //MessageBox(0, tempBuf, NULL, MB_OK);

	// Get module information so that we can avoid searching our own memory pages.
	MODULEINFO ModuleInformation;
	HMODULE hDll = GetModuleHandle("HDLLib.dll");
	if( hDll == NULL )
	{
		sprintf(tempBuf, "Error %d when trying to get module handle.", GetLastError());
		MessageBox(0, tempBuf, NULL, MB_OK);
		return -1;
	}

	if( GetModuleInformation(GetCurrentProcess(), hDll, &ModuleInformation, sizeof(MODULEINFO)) == 0 )
	{
		sprintf(tempBuf, "Error %d when trying to get module information.", GetLastError());
		MessageBox(0, tempBuf, NULL, MB_OK);
		return -1;
	}

	unsigned int i, j;
	unsigned int unreadStateCount = 0, unreadFlagCount = 0, unreadImageCount = 0, readCount = 0;
	for( i = 0; i < numPages; i++ )
	{
		//sprintf(tempBuf, "Page number: %d\nMemory base: %08x\nMemory state: %08x", i, pPage->mbi.BaseAddress, pPage->mbi.State);
		//MessageBox(0, tempBuf, NULL, MB_OK);

		// Determine if this is our module, meaning there will be a copy because we sent it in.
		//sprintf(tempBuf, "Page number: %d\nPage base: %08x\nModule base: %08x\nPage max: %08x\nModule max: %08x", i, (DWORD)pPage->mbi.BaseAddress, (DWORD)ModuleInformation.lpBaseOfDll, (DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize, (DWORD)ModuleInformation.lpBaseOfDll + (DWORD)ModuleInformation.SizeOfImage);
		//MessageBox(0, tempBuf, NULL, MB_OK);
		if( ((DWORD)pPage->mbi.BaseAddress >= (DWORD)ModuleInformation.lpBaseOfDll) &&
			(((DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize)) <= ((DWORD)ModuleInformation.lpBaseOfDll + (DWORD)ModuleInformation.SizeOfImage))
		{
			// Don't read because it's our image.
			//sprintf(tempBuf, "We found an address in our image, so we skipped reading its memory.");
			//MessageBox(0, tempBuf, NULL, MB_OK);
			unreadImageCount++;
		} else {

			// Figure out if we can read the page
			if ( (pPage->mbi.State == MEM_FREE) || (pPage->mbi.State == MEM_RESERVE))
			{
				// Do nothing, because we can't read it.
				// Damn that unallocated memory.
				unreadStateCount++;
			} else {
				// The page is committed, so check for flags making it unreadable.
				int IsUnreadable = 0;
				IsUnreadable |= (pPage->mbi.Protect & PAGE_WRITECOPY);
				IsUnreadable |= (pPage->mbi.Protect & PAGE_EXECUTE);
				IsUnreadable |= (pPage->mbi.Protect & PAGE_GUARD);
				IsUnreadable |= (pPage->mbi.Protect & PAGE_NOACCESS);

				//sprintf(tempBuf, "Page number: %d\nMemory base: %08x\nProtect status: %08x", i, pPage->mbi.BaseAddress, pPage->mbi.Protect);
				//MessageBox(0, tempBuf, NULL, MB_OK);

				if( !IsUnreadable ) // We can read the bloody page.
				{
					readCount++;
					switch( dwSearchType)
					{
						// We can search the page!!!
						case SEARCH_TYPE_BYTE:
							for( j = (DWORD)pPage->mbi.BaseAddress; j < ((DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize); j++ )
							{
								if( !memcmp((unsigned char *)j, pSearchValue, dwSearchLength ) ) // Returns zero if equal
								{
									myArray->AppendAddress(j);
								}
							}
							break;
						case SEARCH_TYPE_WORD:
							for( j = (DWORD)pPage->mbi.BaseAddress; j < ((DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize); j+=2 )
							{
								if( !memcmp((unsigned char *)j, pSearchValue, dwSearchLength ) ) // Returns zero if equal
								{
									myArray->AppendAddress(j);
								}
							}
							break;
						case SEARCH_TYPE_DWORD:
							for( j = (DWORD)pPage->mbi.BaseAddress; j < ((DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize); j+=4 )
							{
								if( !memcmp((unsigned char *)j, pSearchValue, dwSearchLength ) ) // Returns zero if equal
								{
									myArray->AppendAddress(j);
								}
							}
							break;
						/*case SEARCH_TYPE_DOUBLE:
							break;*/
						case SEARCH_TYPE_STRING:
							//for( i=(DWORD)IMAGE.ImageInformation.lpBaseOfDll;
							//	 i < ((DWORD)IMAGE.ImageInformation.lpBaseOfDll + IMAGE.ImageInformation.SizeOfImage);
							//	 i++ )
							//char temp[1000];
							//sprintf(temp, "%d / %d", (DWORD)si.lpMinimumApplicationAddress, (DWORD)si.lpMaximumApplicationAddress);
							//for( i = (DWORD)si.lpMinimumApplicationAddress; i < (DWORD)si.lpMaximumApplicationAddress; i++ )
							//for( i = 0x400000; i < 0x7FFFFFFF; i++ )
							for( j = (DWORD)pPage->mbi.BaseAddress; j < ((DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize - dwSearchLength); j++ )
							{
								if (!memcmp((char *)(j),(const char *)pSearchValue, dwSearchLength))
								{
									//HexDumpMemory((char *)(PhysicalAddress+i),(char *)(PhysicalAddress+i),(MemoryLength-i<256)?MemoryLength-i:256);
									myArray->AppendAddress(j);
								}
							}
							break;
						case SEARCH_TYPE_FINGERPRINT:
							if (LocateFingerPrint((char *)pSearchValue,
												  dwSearchLength,
												  (DWORD)pPage->mbi.BaseAddress,
												  (DWORD)pPage->mbi.BaseAddress + (DWORD)pPage->mbi.RegionSize - dwSearchLength) )
							{
								//
								//MessageBox(0, "Found the pattern.", NULL, MB_OK);
							} else {
								//
							}
							break;
					}
				} else {
					unreadFlagCount++;
				}
			}
		}

		pPage++;
	}

	//sprintf(tempBuf, "There were %d pages read, %d pages unread due to the memory state, %d pages unread due to flags, and %d pages unread because they were in our image.", readCount, unreadStateCount, unreadFlagCount, unreadImageCount);
	//MessageBox(0, tempBuf, NULL, MB_OK);
	  
   return myArray->GetResultCount();

}


LPWORD PreparePattern(char *String)
{
	struct CharMap {
		char Character;
		int CharValue;
	};

	CharMap HexMap[16] =
	{
		{'0', 0}, {'1', 1},
		{'2', 2}, {'3', 3},
		{'4', 4}, {'5', 5},
		{'6', 6}, {'7', 7},
		{'8', 8}, {'9', 9},
		{'A', 10}, {'B', 11},
		{'C', 12}, {'D', 13},
		{'E', 14}, {'F', 15}
	};

	char *StringPtr = String;
	WORD *ReturnValue = NULL;
	ReturnValue = new WORD[strlen(String)>>1];

	short int Index = 0;
	char TempByte = 0;
	bool FirstChar = true;

	while( *StringPtr )
	{
		for( int i = 0; i < 16; i++ )
		{
			if( *StringPtr == 'x' )
			{
				if( FirstChar )
				{
					FirstChar = false;
					break;
				} else {
					FirstChar = true;
					ReturnValue[Index] = 0;
					Index += 1;
					break;
				}
			}
			else if( *StringPtr == HexMap[i].Character )
			{
				if( FirstChar )
				{
					FirstChar = false;
					ReturnValue[Index] = 0xFF00;
					TempByte = HexMap[i].CharValue;
					TempByte <<= 4;
					break;
				} else {
					FirstChar = true;
					TempByte |= HexMap[i].CharValue;
					ReturnValue[Index] |= TempByte;
					Index += 1;
					break;
				}
			}
		}

		StringPtr += 1;
	}

	return ReturnValue;
}

BOOL CheckPattern(LPBYTE Buffer, LPWORD Pattern, DWORD PatternLen) {

	unsigned char TempByte;

	for (unsigned int i = 0; i < PatternLen; i++) {
		TempByte = LOBYTE(Pattern[i]);

		switch ((Pattern[i] & 0xff00)>>8) {
			//ignore on
			case 0x00:
				break;

			//ignore off
			case 0xff:
				if(TempByte != Buffer[i]) 
					return FALSE;

				break;

			default:
				__assume(0);
		}

	}

	return TRUE;
}

BOOL LocateFingerPrint(char *FingerPrint, unsigned long PatternLen, DWORD BeginningOfSearch, DWORD EndOfSearch)
{

	unsigned long i;
	LPBYTE Buffer = 0;
	LPWORD PatternBuffer = 0;
	
	PatternBuffer = PreparePattern(FingerPrint);
	PatternLen = PatternLen / 2;

	for(i=BeginningOfSearch; i < EndOfSearch; i++) {

		if (CheckPattern((LPBYTE)&Buffer[i], PatternBuffer, PatternLen)) {
			delete[] PatternBuffer;

			myArray->AppendAddress( (DWORD)&Buffer[i] );
			return TRUE;
		}
	}

	delete[] PatternBuffer;

	return FALSE;
}

void HexDumpMemory(char *VirtualAddress,char *PhysicalAddress,int length)
//
// Hex dump the given memory range to stdout.
//
{
	char offsetBuffer[100];
	char memBuffer[100];
	char charBuffer[100];
	char outBuffer[2048];
   int i, j;
   char c;
   
	memset(outBuffer, 0, 2048);

   for (i = 0; i < length; i += 16) {
      sprintf(offsetBuffer, "%08X: ", PhysicalAddress + i); // 10
      for (j = 0; j < 16; j++) {
         if (i+j == length) break;
         if (j == 8) printf("- "); // 2
         sprintf(memBuffer + (j*3), "%02X ", *(unsigned char *) (VirtualAddress + i + j)); // 16 * 3 = 48
      }
      for(j = 0; j < 16; j++) {
         if (i+j == length) break;
         c = *(unsigned char *) (VirtualAddress + i + j);
         if (isprint(c)) {
            sprintf(charBuffer + j, "%c", c);
         }
         else {
            sprintf(charBuffer + j, ".");
         }
      }
      sprintf(outBuffer + strlen(outBuffer), "%s%s%s\n", offsetBuffer, memBuffer, charBuffer);
   }

   MessageBox(0, outBuffer, NULL, MB_OK);
}