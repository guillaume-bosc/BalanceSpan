// Global.h: interface for global data & functions.
//
//////////////////////////////////////////////////////////////////////


// Added by Ramin
#include "SeqTree/SeqTree.h"

// Added by Ramin
#if defined( _FIND_MAX_SEQS )
extern SeqTree * MainSeqTree;
#endif // defined( _FIND_MAX_SEQS )

#if defined( _FIND_CLOSED_SEQS )
extern SequenceList * aSeqList;
#endif
// End of the section added by Ramin

#if !defined(_GLOBAL_H__)
#define _GLOBAL_H__
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <map>

#define DISK_BASED 1

//////////////////////////////////////////////////////////////////////
// variable definition.
//////////////////////////////////////////////////////////////////////
#define RESULTS "results.tmp"
#define STATUS	"status.tmp"
#define FREQUENT "frequent.dat"
#define ERRFILE "error.tmp"

#define ORIGINAL 0
#define DUAL 1

#define TRUE 1
#define FALSE 2

#define DONE 0

extern FILE *gpResultFile;
extern FILE *gpStatusFile;
#if defined( _WRITE_FREQUENT_FILE )
extern FILE *gpFreqFile;
#endif
extern FILE *gpErrFile;

extern int gMAX_PAT_LEN;
extern int gN_ITEMS;
extern double gSUP;
extern int* gnArrLargeCount;

#if defined( _FIND_CLOSED_SEQS )
	extern int * gnResSizeCount;
#endif

extern int gnCustCount;
extern int* buf_idx;
#ifndef DISK_BASED
extern int* bufseq;
#endif

extern struct COUNTER* inter;
extern struct COUNTER* intra;
extern int* inter_freq_idx;
extern int* intra_freq_idx;

extern map<int, string> * dico;

//////////////////////////////////////////////////////////////////////
// function definition.
//////////////////////////////////////////////////////////////////////
FILE *file_open(const char *f_name, const char *mode);
inline void* memalloc(size_t nSize){

	void *mem=0;
	if (nSize>0) {
		mem=malloc(nSize);
		if (mem==0){
			fprintf(gpErrFile, "Fail to allocate memory with size %lu\n", nSize);
			exit(-1);
		}
	}
	return(mem);
}

inline void freemem(void** p){

  if (p!=0 && *p!=0){
    free (*p);
    *p=NULL;
  }
}

void ReportError();
void CreateTimers(int nCount);
void DeleteTimers();
#ifdef WIN32
extern struct _timeb *pTimer;
#define ResetTimer(nTimer) _ftime(pTimer+nTimer);
#else
extern struct timeb *pTimer;
#define ResetTimer(nTimer) ftime(pTimer+nTimer);
#endif

double GetTimeDiff(int nTimer);
int freePDB(struct PROJ_DB * pDB);

//////////////////////////////////////////////////////////////////////
#endif // _GLOBAL_H__
