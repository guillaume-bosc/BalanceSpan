// DiskMemory.cpp: implementation of memory maped file.
//
//////////////////////////////////////////////////////////////////////

#if defined(WIN32)

#include <windows.h>
#include "MemMap.h"
#include "Global.h"

struct mem_map{
  unsigned int m_nCurWnd;
	unsigned int m_nFileSize;
  unsigned int m_nWndSize;

  void* m_pLastAddr;
	void* m_lpMemViewFile;
	HANDLE m_hFile;
	HANDLE m_hMapFile;
};

//////////////////////////////////////////////////////////////////////
// Function prototypes

//////////////////////////////////////////////////////////////////////

struct mem_map* CreateMemMap(const char* mem_file, const unsigned int nWndSize){

	struct mem_map* pMem=(struct mem_map*)memalloc(sizeof(struct mem_map));

	pMem->m_hFile = CreateFile(
		mem_file,  // file name. 
		GENERIC_READ, // access mode.
		0,  // share mode is set to exclusive access by using 0.
		(LPSECURITY_ATTRIBUTES)NULL, // security attributes.
		OPEN_EXISTING,  // how to create.
		FILE_ATTRIBUTE_NORMAL,   // file attributes.
		NULL   // handle to template file.
		);
	if (pMem->m_hFile==INVALID_HANDLE_VALUE) {
		fprintf(gpErrFile, "Failed to open memory file '%s'.\n", mem_file);
		ReportError();
		freemem ((void**) &pMem);
		exit(1);
	}

  pMem->m_nCurWnd = 0;
  pMem->m_nFileSize = GetFileSize (pMem->m_hFile, NULL);
  if (pMem->m_nFileSize <= nWndSize) pMem->m_nWndSize = pMem->m_nFileSize;
  else pMem->m_nWndSize = nWndSize;

  pMem->m_hMapFile = CreateFileMapping(
		pMem->m_hFile, // INVALID_HANDLE_VALUE, // handle to file.
		(LPSECURITY_ATTRIBUTES)NULL, // security attributes.
		PAGE_READONLY, // protection. 
		0,   // high-order DWORD of size.
		pMem->m_nWndSize, // low-order DWORD of size.
		NULL   // object name.
		);
	if (pMem->m_hMapFile==NULL){
		fprintf(gpErrFile, "Failed to open memory map-file for '%s'.\n", mem_file);
		ReportError();
		CloseHandle(pMem->m_hFile);
		freemem ((void**) &pMem);
		exit(1);
	}

	pMem->m_lpMemViewFile = MapViewOfFile(
		pMem->m_hMapFile,   // handle to file-mapping object.
		FILE_MAP_READ,   // access mode.
		0,   // high-order DWORD of offset.
		0,   // low-order DWORD of offset.
		0  // number of bytes to map.
		);

	if (pMem->m_lpMemViewFile==NULL){
		fprintf(gpErrFile, "Failed to create view of memory map-file for '%s'.\n", mem_file);
		ReportError();
		CloseHandle(pMem->m_hMapFile);
		CloseHandle(pMem->m_hFile);
		freemem ((void**) &pMem);
		exit(1);
	}

  if (pMem->m_nFileSize <= nWndSize){ 
    pMem->m_pLastAddr = ((char*)(pMem->m_lpMemViewFile) + pMem->m_nFileSize);
  } else {
    int *dataset = (int*) ((char*)(pMem->m_lpMemViewFile) + pMem->m_nWndSize);
    for (; *dataset!=-2; dataset--);
    pMem->m_pLastAddr = (void*)(dataset+1);
  }

	return pMem;
}

void CloseMemMap(struct mem_map** ppMem){

	if (ppMem!=NULL && *ppMem!=NULL) {
		UnmapViewOfFile((*ppMem)->m_lpMemViewFile);
		CloseHandle((*ppMem)->m_hMapFile);
		CloseHandle((*ppMem)->m_hFile);
	}
	freemem ((void**) ppMem);
}

int GetCurWnd(const struct mem_map* pMem){ return pMem->m_nCurWnd; }
void* GetStartOfMap(const struct mem_map* pMem){ return pMem->m_lpMemViewFile; }
void* GetLastAddrOfMap(const struct mem_map* pMem){ return pMem->m_pLastAddr; }
int GetMemMapFileSize(const struct mem_map* pMem){ return pMem->m_nFileSize; }

#endif // WIN32
//////////////////////////////////////////////////////////////////////
// END
