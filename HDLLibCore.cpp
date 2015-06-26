#include <winsock2.h>
#include "socketstream.h"
#include "mempatch.h"
#include "memorysearch.h"
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define HDLM_PATCHED (WM_APP - 1)

#define PID_LOADDLL				0x00000001
#define PID_LOADRESPONSE		0x00000002
#define PID_UNLOADDLL			0x00000003
#define PID_UNLOADRESPONSE		0x00000004
#define	PID_PATCHMEMORY			0x00000005
#define PID_PATCHRESPONSE		0x00000006
#define	PID_SEARCHMEMORY		0x00000007
#define PID_SEARCHRESPONSE		0x00000008
#define PID_SEARCHNEXT			0x00000009
#define PID_NEXTRESPONSE		0x0000000A
#define PID_GETSEARCHRESULTS	0x0000000B
#define PID_GETSEARCHRESPONSE	0x0000000C
#define PID_GETMEMORY			0x0000000D
#define PID_GETMEMORYRESPONSE	0x0000000E
#define PID_CONNECTED			0xFFFFFFFF

HINSTANCE g_hInstDll;

struct {
	HANDLE	hThread;			// Handle to the thread we create
	HWND	hWndTarget;			// Window handle of the target process
	DWORD	dwThreadID;			// Thread Id of the thread we create for listening
	DWORD	dwTargetThreadID;	// Thread Id of the target's window
	DWORD	dwTargetProcessID;	// Process Id of the target
} PROCINFO;

struct PACKETHEADER {
	DWORD	dwIdentifier;		// The packet message identifier (0x00000000)
	WORD	dwPacketID;			// The ide of the packet
	WORD	dwPacketLength;		// The length of the packet, including header
};

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPMSG lParam)
{
	// Let the Apply function know that it's done applying, by sending WM_QUIT to its thread
	if(lParam->message == HDLM_PATCHED)
		PostThreadMessage(lParam->wParam, WM_QUIT, 0, 0);
	
	return 0;
}

void WINAPI HDLThread(LPVOID)
{
	::WinsockStartup();

	std::string myBuffer;
	Socketstream myStream;
	char tempBuffer[1024];
	char outBuffer[1024];
	DWORD	dwResult;
	DWORD	PatchAddress;
	DWORD	PatchLength;
	DWORD	SearchType;
	DWORD	SearchLength;

	if( myStream.connect( "127.0.0.1", 0x7A69 ) )
	{
		(*(DWORD *)tempBuffer) = PID_CONNECTED;
		myStream.send(tempBuffer, 4);

		while( myStream.can_recv() )
		{
			myStream >> myBuffer;
			//MessageBox(0, myBuffer.c_str(), NULL, MB_OK);

			DWORD dwFirstDW = *(DWORD *)myBuffer.c_str();
			std::string	szDllPath;

			AddressArray *myArray = GetAddressArray();
			switch( dwFirstDW )
			{
				case PID_LOADDLL:
					//MessageBox(0, "PID_LOADDLL", NULL, MB_OK);
					szDllPath.assign((char *)(myBuffer.c_str() + 4));
					//MessageBox(0, szDllPath.c_str(), NULL, MB_OK);
					dwResult = (DWORD)LoadLibrary(szDllPath.c_str());
					(*(DWORD *)tempBuffer) = PID_LOADRESPONSE;
					(*(DWORD *)(tempBuffer + 4)) = dwResult;
					(*(DWORD *)(tempBuffer + 8)) = GetLastError();
					myStream.send(tempBuffer, 12);
					break;
				case PID_UNLOADDLL:
					//MessageBox(0, "PID_UNLOADDLL", NULL, MB_OK);
					dwResult = FreeLibrary((HMODULE)*(DWORD *)(myBuffer.c_str() + 4));
					(*(DWORD *)tempBuffer) = PID_UNLOADRESPONSE;
					(*(DWORD *)(tempBuffer + 4)) = dwResult;
					(*(DWORD *)(tempBuffer + 8)) = GetLastError();
					myStream.send(tempBuffer, 12);
					break;
				case PID_PATCHMEMORY:
					//MessageBox(0, "PID_PATCHMEMORY", NULL, MB_OK);
					PatchAddress = *(DWORD *)(myBuffer.c_str() + 4);
					PatchLength = *(DWORD *)(myBuffer.c_str() + 8);
					memcpy(tempBuffer, myBuffer.c_str() + 12, PatchLength);
					//sprintf(outBuffer, "Patching location 0x%08x with a patch of length %d.", PatchAddress, PatchLength);
					//MessageBox(0, outBuffer, NULL, MB_OK);
					(*(DWORD *)outBuffer) = PID_PATCHRESPONSE;
					/*if( !::PatchMemory(PatchAddress, tempBuffer, PatchLength) )
						MessageBox(0, TEXT("Memory patch failed."), NULL, MB_OK);*/
					(*(DWORD *)(outBuffer + 4)) = ::PatchMemory(PatchAddress, tempBuffer, PatchLength);
					(*(DWORD *)(outBuffer + 8)) = GetLastError();
					myStream.send(outBuffer, 12);
					break;
				case PID_SEARCHMEMORY:
					//MessageBox(0, "PID_SEARCHMEMORY", NULL, MB_OK);
					SearchType = *(DWORD *)(myBuffer.c_str() + 4);
					SearchLength = *(DWORD *)(myBuffer.c_str() + 8);
					memcpy(tempBuffer, myBuffer.c_str() + 12, SearchLength);
					tempBuffer[SearchLength] = '\0';
					//sprintf(outBuffer, "Searching for type %d (%s) in %s with a length of %d", SearchType, tempBuffer, tempBuffer2, SearchLength);
					//MessageBox(0, outBuffer, NULL, MB_OK);
					(*(DWORD *)outBuffer) = PID_SEARCHRESPONSE;
					(*(DWORD *)(outBuffer + 4)) = SearchMemory(SearchType, SearchLength, tempBuffer);
					myStream.send(outBuffer, 8);
					break;
				case PID_SEARCHNEXT:
					SearchType = *(DWORD *)(myBuffer.c_str() + 4);
					SearchLength = *(DWORD *)(myBuffer.c_str() + 8);
					memcpy(tempBuffer, myBuffer.c_str() + 12, SearchLength);
					tempBuffer[SearchLength] = '\0';
					(*(DWORD *)outBuffer) = PID_NEXTRESPONSE;
					(*(DWORD *)(outBuffer + 4)) = SearchNext(SearchType, SearchLength, tempBuffer);
					myStream.send(outBuffer, 8);
					break;
				case PID_GETSEARCHRESULTS:
					if( myArray->GetResultCount() <= 100 ) // Don't return that many results!
					{
						(*(DWORD *)outBuffer) = PID_GETSEARCHRESPONSE;
						(*(DWORD *)(outBuffer + 4)) = myArray->GetResultCount();	// Let them know how many DWORDs to read.
						for( unsigned int k=0; k < myArray->GetResultCount(); k++ )
						{
							if( myArray->Index(k) = 0x902015 )
								continue;
							(*(DWORD *)(outBuffer + 8 + (4*k))) = myArray->Index(k);
						}
						myStream.send(outBuffer, 8 + (4*myArray->GetResultCount()));
					}
					break;
				case PID_GETMEMORY:
					PatchAddress = *(DWORD *)(myBuffer.c_str() + 4); // Address they want to read from.
					PatchLength = *(DWORD *)(myBuffer.c_str() + 8); // Length they want to read.
					//sprintf(tempBuffer, "Address: %08x\nLength: %d", PatchAddress, PatchLength);
					(*(DWORD *)outBuffer) = PID_GETMEMORYRESPONSE;
					(*(DWORD *)(outBuffer + 4)) = PatchAddress;
					(*(DWORD *)(outBuffer + 8)) = PatchLength;
					memcpy(outBuffer + 12, (void *)PatchAddress, PatchLength);
					myStream.send(outBuffer, 12 + PatchLength);
					break;
				default:
					sprintf(tempBuffer, "Unrecognized command sent to HDLLib: 0x%08x", dwFirstDW);
					MessageBox(0, tempBuffer, NULL, MB_OK);
					break;
			}
		}
	} else {
		MessageBox(0, "Unable to find the host, exiting...", NULL, MB_OK);
		ExitThread(0);
	}

	::WinsockCleanup();
	ExitThread(0);

	return;
}

BOOL WINAPI Inject()
{
	if( PROCINFO.hWndTarget == NULL )
		return FALSE;

	PROCINFO.dwTargetThreadID = GetWindowThreadProcessId(PROCINFO.hWndTarget, &PROCINFO.dwTargetProcessID);

	if( PROCINFO.dwTargetThreadID == NULL )
		return FALSE;

	HHOOK loadHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, g_hInstDll, PROCINFO.dwTargetThreadID);

	if( loadHook == NULL )
		return FALSE;

	PostThreadMessage(PROCINFO.dwTargetThreadID, HDLM_PATCHED, GetCurrentThreadId(), 0);

	SetTimer(NULL, 0, 5000, NULL);

	BOOL Status;
	MSG Msg;
	while(Status = GetMessage(&Msg, NULL, 0, 0) && Status != -1) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);

		if(Msg.message == WM_TIMER && Msg.lParam == NULL)
			break; // Timeout
	}

	UnhookWindowsHookEx(loadHook);

	return TRUE;
}

BOOL WINAPI LoadIntoProcessByWindowTitle(LPSTR szWindowTitle)
{
	PROCINFO.hWndTarget = FindWindow(NULL, szWindowTitle);

	return Inject();
}

BOOL WINAPI LoadIntoProcessByWindowClass(LPSTR szWindowClass)
{
	PROCINFO.hWndTarget = FindWindow(szWindowClass, NULL);

	return Inject();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, PVOID fImpLoad)
{
	char *hostFileName = "HostApp.exe";

	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			// The DLL is being mapped into the process' address space.
			g_hInstDll = hInstDll;

			// We don't want notification when other threads attach.
			::DisableThreadLibraryCalls(hInstDll);

			TCHAR myModuleName[MAX_PATH];
			
			if(!lstrcmpi(myModuleName + GetModuleFileName(NULL, myModuleName, MAX_PATH)
				- strlen("HostApp.exe"), hostFileName)) // Then we're in the host file
			{
					//MessageBox(0, TEXT("We're in the host file."), NULL, MB_OK);
					return TRUE; // Don't load the thread.
			} else {
				//MessageBox(0, myModuleName + GetModuleFileName(NULL, myModuleName, MAX_PATH)- strlen("HostApp.exe"), hostFileName, MB_OK);
			}

			// Find the path to the HDL
			GetModuleFileName(hInstDll, myModuleName, MAX_PATH);
			// Load another copy so that we don't lose it when the host process exits.
			LoadLibrary(myModuleName);

			PROCINFO.hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HDLThread, NULL, 0, &PROCINFO.dwThreadID);
			if( PROCINFO.hThread == 0 )
			{
				// We've failed.
				FreeLibrary(g_hInstDll);
				return FALSE;
			}
			CloseHandle(PROCINFO.hThread);
			break;

		case DLL_PROCESS_DETACH:
			// The DLL is being unmapped from the process' address space.
			break;

		default:
			// We can never get here...
			__assume(0);
	}

	return(TRUE);
}