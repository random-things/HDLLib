#include <windows.h>

#define TOTALVMRESERVE		0x00100000
#define PAGESIZE			0x1000

struct VMOBJECT
{
	MEMORY_BASIC_INFORMATION	mbi;
	char			szObjType[12];
	char			szModule[MAX_PATH];
	char			szSection[IMAGE_SIZEOF_SHORT_NAME];
	BOOL			bNew;
} ;

typedef	struct VMOBJECT *LPVMOBJECT ;

class CMemoryWalker {
	private:
		LPVOID m_lpWalk;
		int	nCnt;
	public:
		CMemoryWalker();
		virtual ~CMemoryWalker();

		int memWalk();
		LPVMOBJECT mGetPageList(int *pCnt);
};


CMemoryWalker::CMemoryWalker()
{
	m_lpWalk = NULL;
	nCnt = 0;
}

CMemoryWalker::~CMemoryWalker()
{
	BOOL bRet;
	DWORD dwSize;
    if (m_lpWalk)
	{
		//Decommit page region
		dwSize = 0;
		bRet = VirtualFree(m_lpWalk, dwSize, MEM_DECOMMIT);
		bRet = VirtualFree(m_lpWalk, dwSize, MEM_RELEASE);
	}
}

//This is a rewrite of the procedure from 
//MS SDK in a way which ports seealessly between C and C++
int CMemoryWalker::memWalk()
{
    LPVMOBJECT	lpList;
    LPVOID		lpMem, lpUncommited;
    SYSTEM_INFO	si;
	DWORD dwSize, dwIndex;
	HANDLE hOtherProcess = GetCurrentProcess();

    /* if pointer exists, reset to no commit */
    if (m_lpWalk)
	{
		//Decommit page region
		dwSize = 0;
		if(!VirtualFree (m_lpWalk, dwSize, MEM_DECOMMIT))
			return 0;
	} else {
		/* else perform initial reserve */
		dwSize = TOTALVMRESERVE; //100,000
		m_lpWalk = VirtualAlloc(NULL, //Any Address
			TOTALVMRESERVE,
			MEM_RESERVE,	//No alloc, hold for LocalAlloc
			PAGE_NOACCESS);	//DO a GP fault for r,w or x
		if(m_lpWalk == NULL) return 0;
	    lpList = (LPVMOBJECT)m_lpWalk;
		lpUncommited = (LPVOID)m_lpWalk;
	}

    /* initialize list pointer to beginning of walker list */
	lpUncommited = (LPVOID)m_lpWalk;
    lpList = (LPVMOBJECT)m_lpWalk;

    /* Get maximum address range from system info */
    GetSystemInfo(&si);

    /* walk process addresses */
	lpMem = 0;
	dwIndex = 0;
    while (lpMem < si.lpMaximumApplicationAddress)
	{
		//Check for out of memory
		if(	((int)lpList + 4096) >= ((int)m_lpWalk + TOTALVMRESERVE) )
		{
			//Out of memory
			return 0;
		}
		//Commit the pages for retaining mem info
	    if( (lpList + sizeof(VMOBJECT)) >= lpUncommited ) 
		{
			if (VirtualAlloc(
				lpUncommited,
				4096,
				MEM_COMMIT,
				PAGE_READWRITE) == NULL )
			{
				//Reserved pages got used elsewhere
				return 0;
			}
			lpUncommited = (LPVOID)((DWORD)lpList+ (DWORD)4096);
		}
		//Init info for this page
	    *lpList->szObjType = 0;
	    *lpList->szModule = 0;
	    *lpList->szSection = 0;
	    lpList->bNew = 0;

	    VirtualQueryEx(
			hOtherProcess,
			lpMem,
			&(lpList->mbi),
			sizeof(MEMORY_BASIC_INFORMATION));
	    /* increment lpMem to next region of memory */
	    lpMem = (LPVOID)((DWORD)lpList->mbi.BaseAddress +
			     (DWORD)lpList->mbi.RegionSize);
	    lpList++;
		++dwIndex;
    }

	dwSize = ((DWORD)lpList - (DWORD)m_lpWalk);
    nCnt =  dwSize/sizeof(VMOBJECT);
    /* return number of items in list */
    return (nCnt);
}


LPVMOBJECT CMemoryWalker::mGetPageList(int *pCnt)
{
    LPVMOBJECT	lpList;
    lpList = (LPVMOBJECT)m_lpWalk;
	if(pCnt != NULL)
		*pCnt = nCnt;
	return lpList;
}