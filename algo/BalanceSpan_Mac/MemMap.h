// MemMap.h: interface for memory maped file.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MEMMAP_H__)
#define _MEMMAP_H__

// Forward declaration.
struct mem_map;

struct mem_map* CreateMemMap(const char* mem_file, const unsigned int nWndSize);
void CloseMemMap(struct mem_map** ppMem);
int GetCurWnd(const struct mem_map* pMem);
void* GetStartOfMap(const struct mem_map* pMem);
void* GetLastAddrOfMap(const struct mem_map* pMem);
int GetMemMapFileSize(const struct mem_map* pMem);


//////////////////////////////////////////////////////////////////////
#endif // _MEMMAP_H__
