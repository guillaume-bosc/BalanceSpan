// Global.cpp: implementation of global data & functions.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include "Global.h"
#include "ProjDB.h"

#if defined(WIN32)
#include <windows.h>
#else
#include <errno.h>
#endif // WIN32
//////////////////////////////////////////////////////////////////////
// variables.
//////////////////////////////////////////////////////////////////////
FILE *gpResultFile = NULL; // time taken, # of freq items and # of each large item.
FILE *gpStatusFile = NULL; // on going progress status of program.
#if defined( _WRITE_FREQUENT_FILE )
FILE *gpFreqFile=NULL; // frequent items found.
#endif
FILE *gpErrFile = NULL; // file handle containing data set to be mined.

int gMAX_PAT_LEN = 30; // maximum length of a frequent sequence.
int gN_ITEMS = 10000; // # of items in the database.
double gSUP = 0.0; // requested support for frequent patterns.
int* gnArrLargeCount = NULL; // # of large patterns of different lengths.
#if defined( _FIND_CLOSED_SEQS )
int * gnResSizeCount = NULL; // # of patterns of different lengths in the resultset.
#endif
int gnCustCount = 0; // number of customers in database.
int* buf_idx = 0;
#ifndef DISK_BASED
int* bufseq=0;
#endif

struct COUNTER* inter = NULL;
struct COUNTER* intra = NULL;
int* inter_freq_idx = NULL;
int* intra_freq_idx = NULL;

map<int, string> * dico = NULL;

#if defined(WIN32)
struct _timeb *pTimer=0;
#else
struct timeb *pTimer = 0;
#endif // WIN32
//////////////////////////////////////////////////////////////////////
// functions.
//////////////////////////////////////////////////////////////////////
FILE *file_open(const char *f_name, const char *mode) {

	FILE *f = fopen(f_name, mode);

	if (f == 0) {
		if (gpErrFile != NULL)
			fprintf(gpErrFile, "Fail to open file %s with mode %s\n", f_name,
					mode);
		else
			printf("Fail to open file %s with mode %s\n", f_name, mode);
		exit(-1);
	}
	return (f);
}

#if defined(WIN32)
void CreateTimers(int nCount) {
	pTimer=(struct _timeb*)memalloc(sizeof(struct _timeb)*nCount);
}
double GetTimeDiff(int nTimer) {

	struct _timeb endTime;
	_ftime(&endTime); // record end time.
	return (double(endTime.time - pTimer[nTimer].time) +
			(endTime.millitm - pTimer[nTimer].millitm) / 1000.0);
}

void ReportError() {

	char *lpszError=NULL;

	/* get formatted error message from system */
	if (!FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError (),
					MAKELONG (MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), 0),
					(LPTSTR)&lpszError,
					0,
					NULL))
	return;

	fprintf(gpErrFile, "%s\n", lpszError);
	LocalFree(lpszError);
}

#else
void CreateTimers(int nCount) {
	pTimer = (struct timeb*) memalloc(sizeof(struct timeb) * nCount);
}

double GetTimeDiff(int nTimer) {

	struct timeb endTime;
	ftime(&endTime); // record end time.
	return (double(endTime.time - pTimer[nTimer].time)
			+ (endTime.millitm - pTimer[nTimer].millitm) / 1000.0);
}

void ReportError() {

	fprintf(gpErrFile, "%s\n", strerror(errno));
}
#endif // WIN32
void DeleteTimers() {
	freemem((void**) &pTimer);
}

int freePDB(struct PROJ_DB * pDB) {
	int mem = 0;
	int j = 0;
	mem += (pDB->m_nPatLen * sizeof(int));
	freemem((void**) &(pDB->m_pnPat));
	for (j = 0; j < pDB->m_nSup; j++) {
		if (pDB->m_pProjSeq[j].m_nProjCount > 1) {
			mem += (pDB->m_pProjSeq[j].m_nProjCount * sizeof(int*));
			freemem((void**) &(pDB->m_pProjSeq[j].m_ppSeq));
		}
	}

	mem += (pDB->m_nMaxSup * sizeof(struct PROJ_SEQ));
	freemem((void**) &(pDB->m_pProjSeq));

	return mem;
}

//////////////////////////////////////////////////////////////////////
// END
