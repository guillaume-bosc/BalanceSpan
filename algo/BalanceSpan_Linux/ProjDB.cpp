// ProjDB.cpp: implementation of projected database structure.
//
//////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <iostream>

#include "ProjDB.h"
#include "Global.h"
#include "MemMap.h"

//////////////////////////////////////////////////////////////////////
// variables.
//////////////////////////////////////////////////////////////////////
int n_proj_db = 0; // number of projected sequences processed.
int n_max_mem = 0; // maximum memory usage by projected databases.
int n_total_mem = 0; // total memory usage by projected databases.
struct mem_map *pDatasetMemMap = NULL;

//////////////////////////////////////////////////////////////////////
// functions.
//////////////////////////////////////////////////////////////////////
int InitProjDB(const char* filename) {

	inter = (struct COUNTER*) memalloc(sizeof(struct COUNTER) * gN_ITEMS);
	intra = (struct COUNTER*) memalloc(sizeof(struct COUNTER) * gN_ITEMS);
	inter_freq_idx = (int*) memalloc(sizeof(int) * gN_ITEMS);
	intra_freq_idx = (int*) memalloc(sizeof(int) * gN_ITEMS);

	pDatasetMemMap = CreateMemMap(filename, 1536 * 1024 * 1024);
	return GetMemMapFileSize(pDatasetMemMap);
}

void CloseProjDB() {

	freemem((void**) &inter);
	freemem((void**) &intra);
	freemem((void**) &inter_freq_idx);
	freemem((void**) &intra_freq_idx);
	CloseMemMap(&pDatasetMemMap);
}

///////////////////////////////////////////////
// Procs added by Ramin
///////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

struct PROJ_DB* make_projdb_from_org_dataset(const double dSupport,
		int* pnFreqCount) {

	int i = 0, j = 0, nSize = 0, nCount = 0, nPatLen = 0, nMaxSeqLen = 0,
			nSeqLen = 0;
	int *d = 0, *dataset = (int*) GetStartOfMap(pDatasetMemMap);
	int *lastAddr = (int*) GetLastAddrOfMap(pDatasetMemMap);
	struct PROJ_DB* proj_db = NULL;
	struct PROJ_DB* tempDB = NULL;
	struct PROJ_SEQ* tempSeq = NULL;
	struct COUNTER* pCnter = 0;

	*pnFreqCount = 0;

	//fprintf(gpStatusFile, "\tCounting 1-Item sets ... \n");
	fflush(gpStatusFile);
	ResetTimer(1);

	memset(inter_freq_idx, 0, sizeof(int) * gN_ITEMS);
	memset(inter, 0, sizeof(struct COUNTER) * gN_ITEMS);

	// Scaning DB to count the frequent items.
	for (nCount = 0; dataset < lastAddr; dataset++) {
		nCount++;
		nSeqLen = nPatLen = 0;
		for (; *dataset != -2; dataset++) {
			for (; *dataset != -1; dataset++) {
				// eat up consecutive identical numbers.
				while (dataset[0] == dataset[1])
					dataset++;
				nPatLen++;
				nSeqLen++;
				pCnter = inter + (*dataset);
				if (pCnter->s_id < nCount) {
					pCnter->s_id = nCount;
					pCnter->count++;

					if (dataset[2] != -2)
						inter_freq_idx[*dataset]++;
				}
				nSeqLen++;
			}
			nSeqLen++;
		}
		if (gMAX_PAT_LEN < nPatLen)
			gMAX_PAT_LEN = nPatLen;
		if (nMaxSeqLen < nSeqLen)
			nMaxSeqLen = nSeqLen;
	}

	gnCustCount = nCount;

	gSUP = (nCount * dSupport);
	gMAX_PAT_LEN += 2; // since we don't use the first and last index.
	gnArrLargeCount = (int*) memalloc(sizeof(int) * gMAX_PAT_LEN);
	memset(gnArrLargeCount, 0, sizeof(int) * gMAX_PAT_LEN);

	buf_idx = (int*) memalloc(sizeof(int) * nMaxSeqLen);
	memset(buf_idx, 0, sizeof(int) * nMaxSeqLen);

///////////////////////////////
// Changed by Ramin

	for (i = 0; i < gN_ITEMS; i++) {
		pCnter = inter + i;

		if (pCnter->count >= gSUP) {

			OutputPattern(&i, 1, pCnter->count);

		}
	}

///////////////////////////////

	// If there are 1 item frequent seqs.
	if (gnArrLargeCount[1] > 0) {
		ResetTimer(1);
		//fprintf(gpStatusFile, "\tCreating projection DBs ... ");
		fflush(gpStatusFile);
		*pnFreqCount = gnArrLargeCount[1];
		n_total_mem += (nSize = (gnArrLargeCount[1] * sizeof(struct PROJ_DB)));
		proj_db = (struct PROJ_DB*) memalloc(nSize);

		for (nCount = i = 0; i < gN_ITEMS; i++) {
			if (inter[i].count >= gSUP) {

				proj_db[nCount].m_nPatLen = 1;
				proj_db[nCount].m_pnPat = (int*) i;

				proj_db[nCount].m_nMaxSup = (*(inter + i)).count;

				n_total_mem += (nSize = (inter_freq_idx[i]
						* sizeof(struct PROJ_SEQ)));
				proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*) memalloc(nSize);
				memset(proj_db[nCount].m_pProjSeq, 0, nSize);

				proj_db[nCount].m_nVer = -1;
				proj_db[nCount].m_nSup = 0;
				proj_db[nCount].NumOfItems = 0;
				proj_db[nCount].ItemIsIntra = false;
				proj_db[nCount].Item = i;
				proj_db[nCount].isProjected = FALSE;
				proj_db[nCount].isDone = FALSE;
				proj_db[nCount].dual = NULL;
				// Set the set of the IDs of the sequences
				proj_db[nCount].SeqId = new deque<int>();

				inter_freq_idx[i] = nCount;
				nCount++;
			} else
				inter_freq_idx[i] = -1;
		}

		// scan database again, do projection
		dataset = (int*) GetStartOfMap(pDatasetMemMap);

		for (nCount = 0; dataset < lastAddr; dataset++) {
			nCount++;
			for (; *dataset != -2; dataset++) {
				for (; *dataset != -1; dataset++) {
					// eat up consecutive identical numbers.
					while (dataset[0] == dataset[1])
						dataset++;
					i = inter_freq_idx[*dataset];

					if (i != -1
							&& (proj_db[i].SeqId->size() == 0
									|| (proj_db[i].SeqId->size() > 0
											&& proj_db[i].SeqId->back()
													!= nCount))) {
						proj_db[i].SeqId->push_back(nCount);
					}

					// If this is the last item in Seq, or is not frequent, or an instance of this item has been seen in this seq before.
					if (dataset[2] == -2 || i < 0
							|| proj_db[i].m_nVer >= nCount)
						continue;

					// Pointer to Proj_DB for this item.
					tempDB = proj_db + i;
					// Pointer to the next available Seq in this DB.
					tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
					// Last sequence contributed to this DB.
					tempDB->m_nVer = nCount;
					(*tempDB).NumOfItems += (lastAddr - dataset);

					tempDB->m_nSup++;

					/*
					 * Merge two dual BD
					 */
					if ((*dataset) % 2 == 1) {
						tempDB->status = DUAL;

						if (i != 0
								&& (proj_db + i - 1)->Item
										== ((*dataset) - 1)) {

							// If there is a dual BD
							tempDB->dual = (proj_db + i - 1);
							tempDB->dual->dual = tempDB;

						} else {
							tempDB->dual = NULL;
						}
					} else {
						tempDB->status = ORIGINAL;
					}

					for (d = dataset + 1, buf_idx[0] = int(d), j = 1;
							d[2] != -2; d++) {
						if (*d == *dataset && d[1] != -1)
							buf_idx[j++] = int(d + 1);
					}
					tempSeq->m_nProjCount = j;
					if (j == 1) {
						tempSeq->m_ppSeq = (int**) buf_idx[0];
					} else {
						n_total_mem += (nSize = sizeof(int*) * j);
						tempSeq->m_ppSeq = (int**) memalloc(nSize);
						memcpy(tempSeq->m_ppSeq, buf_idx, nSize);
					}

				}
			}
		}
	}

	n_max_mem = n_total_mem;
	return proj_db;
}

struct PROJ_DB* make_projdb_from_projected_db(const struct PROJ_DB* pDB,
		int* pnFreqCount) {

	struct PROJ_DB* proj_db = NULL;
	int i = 0, j = 0, k = 0, l = 0, nSize = 0, nCount = 0, nProjCnt = 0;
	int *d = 0, *dataset = 0;
	struct PROJ_DB* tempDB = NULL;
	struct PROJ_SEQ* tempSeq = NULL;
	struct COUNTER* pCnter = 0;

	int *lastAddr = (int*) GetLastAddrOfMap(pDatasetMemMap);

	*pnFreqCount = 0; // number of frequent items
	//PrintProjDBs(pDB, 1);

	// scan database once, find inter- and intra- element frequent items
	memset(intra, 0, sizeof(struct COUNTER) * gN_ITEMS);
	memset(inter, 0, sizeof(struct COUNTER) * gN_ITEMS);

	for (nCount = 0; nCount < pDB->m_nSup;) {
		nCount++;
		nProjCnt = pDB->m_pProjSeq[nCount - 1].m_nProjCount;
		for (i = 0; i < nProjCnt; i++) {
			if (nProjCnt == 1)
				dataset = (int*) pDB->m_pProjSeq[nCount - 1].m_ppSeq;
			else
				dataset = pDB->m_pProjSeq[nCount - 1].m_ppSeq[i];

			// counting intra-element items.
			for (; *dataset != -1; dataset++) {
				// eat up consecutive identical numbers.
				while (dataset[0] == dataset[1])
					dataset++;
				pCnter = intra + (*dataset);
				if (pCnter->s_id < nCount) {
					pCnter->count++;
					pCnter->s_id = nCount;

				}
			}
			// for inter we only need to count the longest instance
			// of a projected sequence. ie. the first one (i==0).
			if (i != 0)
				continue;
			for (dataset++; *dataset != -2; dataset++) {
				// counting inter-element items.
				for (; *dataset != -1; dataset++) {
					// eat up consecutive identical numbers.
					while (dataset[0] == dataset[1])
						dataset++;
					pCnter = inter + (*dataset);
					if (pCnter->s_id < nCount) {
						pCnter->count++;
						pCnter->s_id = nCount;
					}
				}
			}
		}
	}

	for (j = k = i = 0; i < gN_ITEMS; i++) {
		if (intra[i].count >= gSUP)
			j++;
		if (inter[i].count >= gSUP)
			k++;
	}

	if ((j + k) > 0) {
		*pnFreqCount = (j + k);
		n_total_mem += (nSize = (*pnFreqCount * sizeof(struct PROJ_DB)));
		proj_db = (struct PROJ_DB*) memalloc(nSize);
		memset(inter_freq_idx, -1, sizeof(int) * gN_ITEMS);
		memset(intra_freq_idx, -1, sizeof(int) * gN_ITEMS);
		n_total_mem += sizeof(int)
				* ((*pnFreqCount * (pDB->m_nPatLen + 1)) + k);

		for (j = sizeof(int) * (pDB->m_nPatLen + 1), nCount = i = 0;
				i < gN_ITEMS; i++) {
			if (intra[i].count >= gSUP) {
				proj_db[nCount].m_nPatLen = (pDB->m_nPatLen + 1);
				proj_db[nCount].m_pnPat = (int*) memalloc(j);
				if (pDB->m_nPatLen == 1) {
					proj_db[nCount].m_pnPat[0] = (int) pDB->m_pnPat;
				} else {
					memcpy(proj_db[nCount].m_pnPat, pDB->m_pnPat,
							j - sizeof(int));
				}
				proj_db[nCount].m_pnPat[pDB->m_nPatLen] = i;

				OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen + 1,
						intra[i].count);

				proj_db[nCount].m_nMaxSup = intra[i].count;
				n_total_mem +=
						(nSize = intra[i].count * sizeof(struct PROJ_SEQ));
				proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*) memalloc(nSize);
				memset(proj_db[nCount].m_pProjSeq, 0, nSize);

				proj_db[nCount].m_nVer = -1;
				proj_db[nCount].m_nSup = 0;
				proj_db[nCount].NumOfItems = 0;
				proj_db[nCount].ItemIsIntra = true;
				proj_db[nCount].Item = i;
				proj_db[nCount].isProjected = FALSE;
				proj_db[nCount].isDone = FALSE;
				proj_db[nCount].dual = NULL;
				// Set the set of the IDs of the sequences
				proj_db[nCount].SeqId = new deque<int>();

				proj_db[nCount].status = pDB->status;

				intra_freq_idx[i] = nCount;
				nCount++;

			}

			if (inter[i].count >= gSUP) {
				proj_db[nCount].m_nPatLen = (pDB->m_nPatLen + 2);
				proj_db[nCount].m_pnPat = (int*) memalloc(sizeof(int) + j);
				if (pDB->m_nPatLen == 1) {
					proj_db[nCount].m_pnPat[0] = (int) pDB->m_pnPat;
				} else {
					memcpy(proj_db[nCount].m_pnPat, pDB->m_pnPat,
							j - sizeof(int));
				}
				proj_db[nCount].m_pnPat[pDB->m_nPatLen] = -1;
				proj_db[nCount].m_pnPat[pDB->m_nPatLen + 1] = i;

				OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen + 2,
						inter[i].count);

				proj_db[nCount].m_nMaxSup = inter[i].count;
				n_total_mem +=
						(nSize = inter[i].count * sizeof(struct PROJ_SEQ));
				proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*) memalloc(nSize);
				memset(proj_db[nCount].m_pProjSeq, 0, nSize);

				proj_db[nCount].m_nVer = -1;
				proj_db[nCount].m_nSup = 0;
				proj_db[nCount].NumOfItems = 0;
				proj_db[nCount].ItemIsIntra = false;
				proj_db[nCount].Item = i;
				proj_db[nCount].isProjected = FALSE;
				proj_db[nCount].isDone = FALSE;
				proj_db[nCount].dual = NULL;
				// Set the set of the IDs of the sequences
				proj_db[nCount].SeqId = new deque<int>();

				proj_db[nCount].status = pDB->status;

				inter_freq_idx[i] = nCount;
				nCount++;
			}
		}

		// scan database again, do projection
		for (nCount = 0; nCount < pDB->m_nSup;) {
			nCount++;
			nProjCnt = pDB->m_pProjSeq[nCount - 1].m_nProjCount;
			for (i = 0; i < nProjCnt; i++) {
				if (nProjCnt == 1)
					dataset = (int*) pDB->m_pProjSeq[nCount - 1].m_ppSeq;
				else
					dataset = pDB->m_pProjSeq[nCount - 1].m_ppSeq[i];

				// counting intra-element items.
				for (; *dataset >= 0; dataset++) {
					// eat up consecutive identical numbers.
					while (dataset[0] == dataset[1])
						dataset++;

					j = intra_freq_idx[*dataset];

					if (j != -1
							&& (proj_db[j].SeqId->size() == 0
									|| (proj_db[j].SeqId->size() > 0
											&& proj_db[j].SeqId->back()
													!= pDB->SeqId->at(
															nCount - 1)))) {
						proj_db[j].SeqId->push_back(pDB->SeqId->at(nCount - 1));
					}

					if (dataset[2] == -2 || j < 0
							|| proj_db[j].m_nVer >= nCount)
						continue;

					tempDB = proj_db + j;
					tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
					tempDB->m_nVer = nCount;

					(*tempDB).NumOfItems += (lastAddr - dataset);

					for (buf_idx[0] = int(dataset + 1), l = 1, j = i;
							j < nProjCnt; j++) {
						if (j == i)
							d = dataset + 1;
						else
							d = pDB->m_pProjSeq[nCount - 1].m_ppSeq[j];
						while (*d != -1 && d[2] != -2 && *d != *dataset)
							d++;
						if (d[2] != -2 && *d == *dataset && d[1] != -1)
							buf_idx[l++] = int(d + 1);
					}
					tempDB->m_nSup++;
					tempSeq->m_nProjCount = l;
					if (l == 1) {
						tempSeq->m_ppSeq = (int**) buf_idx[0];
					} else {
						n_total_mem += (nSize = sizeof(int*) * l);
						tempSeq->m_ppSeq = (int**) memalloc(nSize);
						memcpy(tempSeq->m_ppSeq, buf_idx, nSize);
					}

				} // end for counting intra-element items.

				// for inter we only need to work with the longest instance
				// of a projected sequence. ie. the first one (i==0).
				if (i != 0)
					continue;
				for (dataset++; *dataset != -2; dataset++) {
					// counting inter-element items.
					for (; *dataset != -1; dataset++) {
						// eat up consecutive identical numbers.
						while (dataset[0] == dataset[1])
							dataset++;
						j = inter_freq_idx[*dataset];

						if (j != -1
								&& (proj_db[j].SeqId->size() == 0
										|| (proj_db[j].SeqId->size() > 0
												&& proj_db[j].SeqId->back()
														!= pDB->SeqId->at(
																nCount - 1)))) {
							proj_db[j].SeqId->push_back(
									pDB->SeqId->at(nCount - 1));
						}

						if (dataset[2] == -2 || j < 0
								|| proj_db[j].m_nVer >= nCount)
							continue;

						tempDB = proj_db + j;
						tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
						tempDB->m_nVer = nCount;

						(*tempDB).NumOfItems += (lastAddr - dataset);

						for (d = dataset + 1, buf_idx[0] = int(d), l = 1;
								d[2] != -2; d++) {
							if (*d == *dataset && d[1] != -1)
								buf_idx[l++] = int(d + 1);
						}
						tempDB->m_nSup++;
						tempSeq->m_nProjCount = l;
						if (l == 1) {
							tempSeq->m_ppSeq = (int**) buf_idx[0];
						} else {
							n_total_mem += (nSize = sizeof(int*) * l);
							tempSeq->m_ppSeq = (int**) memalloc(nSize);
							memcpy(tempSeq->m_ppSeq, buf_idx, nSize);
						}

					}
				} // end for counting inter-element items.
			} // end of all projection instances of a sequence.
		} // end of all sequences.
	} // enf if number of projections is greater than 0.

	if (n_max_mem < n_total_mem)
		n_max_mem = n_total_mem;
	if (pDB->m_nPatLen > 1) {
		n_total_mem -= (pDB->m_nPatLen * sizeof(int));
		freemem((void**) &(pDB->m_pnPat));
	}

	if (pDB->SeqId != NULL) {
		delete (pDB->SeqId);
	}
	for (i = 0; i < pDB->m_nSup; i++) {

		if (pDB->m_pProjSeq[i].m_nProjCount > 1) {
			n_total_mem -= (pDB->m_pProjSeq[i].m_nProjCount * sizeof(int*));
			freemem((void**) &(pDB->m_pProjSeq[i].m_ppSeq));
		}
	}
	n_total_mem -= (pDB->m_nMaxSup * sizeof(struct PROJ_SEQ));
	freemem((void**) &(pDB->m_pProjSeq));

	// PrintProjDBs(proj_db, *pnFreqCount);
	return proj_db;
}

//this part is copied from make_projdb_from_projected_db's tail part.
#if defined (_ANOTHER_CLOSED_APPROACH)
void clean_projcted_db(const struct PROJ_DB* pDB, int* pnFreqCount) {
	int i;

	if (n_max_mem < n_total_mem)
		n_max_mem = n_total_mem;
	if (pDB->m_nPatLen > 1) {
		n_total_mem -= (pDB->m_nPatLen * sizeof(int));
		freemem((void**) &(pDB->m_pnPat));
	}
	for (i = 0; i < pDB->m_nSup; i++) {

		if (pDB->m_pProjSeq[i].m_nProjCount > 1) {
			n_total_mem -= (pDB->m_pProjSeq[i].m_nProjCount * sizeof(int*));
			freemem((void**) &(pDB->m_pProjSeq[i].m_ppSeq));
		}
	}
	n_total_mem -= (pDB->m_nMaxSup * sizeof(struct PROJ_SEQ));
	freemem((void**) &(pDB->m_pProjSeq));
}
#endif

//////////////////////////////////////////////////////////////////////
// Local functions.
//////////////////////////////////////////////////////////////////////
void PrintProjDBs(const struct PROJ_DB *proj_db, const int nCount) {

	int i = 0, j = 0, k = 0, l = 0, nProjCnt = 0, *dataset = 0;

	printf("\nProjected databases:\n");
	for (i = 0; i < nCount; i++) {
		printf("Proj. DB. for (");
		if (proj_db[i].m_nPatLen == 1)
			printf("%d", int(proj_db[i].m_pnPat));
		else
			for (j = 0; j < proj_db[i].m_nPatLen; j++) {
				if (proj_db[i].m_pnPat[j] == -1)
					printf(")");
				else if (j > 0 && proj_db[i].m_pnPat[j - 1] == -1)
					printf(" (%d", proj_db[i].m_pnPat[j]);
				else if (j > 0)
					printf(" %d", proj_db[i].m_pnPat[j]);
				else
					printf("%d", proj_db[i].m_pnPat[j]);
			}
		printf("\n ");

		printf(
				"NumOfItems = %lu m_nSupport= %d  maxSupport= %d totalmem= %d maxmem= %d\n",
				proj_db[i].NumOfItems, proj_db[i].m_nSup, proj_db[i].m_nMaxSup,
				n_total_mem, n_max_mem);

		return;

		for (j = 0; j < proj_db[i].m_nSup; j++) {
			nProjCnt = proj_db[i].m_pProjSeq[j].m_nProjCount;
			for (k = 0; k < nProjCnt; k++) {
				if (nProjCnt == 1)
					dataset = (int*) proj_db[i].m_pProjSeq[j].m_ppSeq;
				else
					dataset = proj_db[i].m_pProjSeq[j].m_ppSeq[k];

				for (l = 0; dataset[l] != -2; l++) {
					if (dataset[l] == -1)
						printf(")");
					else if (l > 0 && dataset[l - 1] == -1)
						printf(" (%d", dataset[l]);
					else if (l > 0)
						printf(" %d", dataset[l]);
					else
						printf("%d", dataset[l]);
				} // end of an instance for a projected sequence.
				printf("\n ");
			} // end of all instances for a projected sequence.
			printf("\n ");
		} // end of all projected sequences.
	}
}

//inline bool OutputPattern( const int *const pat, const int nPatLen, const int nSup )
bool OutputPattern(const int * const pat, const int nPatLen, const int nSup) {
	bool Result = true;
	int i = 0, l = 0;

	for (; i < nPatLen; i++)
		if (pat[i] != -1)
			l++;

	gnArrLargeCount[l]++;

	return Result;
}
//////////////////////////////////////////////////////////////////////
// END

// Added by Ramin
void Test() {
	int * dataset = (int*) GetStartOfMap(pDatasetMemMap);
	int * lastAddr = (int*) GetLastAddrOfMap(pDatasetMemMap);
	int * RecStart;
	FILE *testFile = NULL;
	testFile = file_open("Test.txt", "w");
	long NumOfRecs = 0;
	long NumOfItems = 0;
	long NumOfItemSets = 0;

	for (int nCount = 0; dataset < lastAddr; dataset++, nCount++) {
		RecStart = dataset;
		NumOfRecs++;
		// For each sequence
		for (; *dataset != -2; dataset++) {
			NumOfItemSets++;
			// For each Itemset
			for (; *dataset != -1; dataset++) {
				NumOfItems++;
				fprintf(testFile, " %d ", *dataset);
			}
			fprintf(testFile, " | ");
		}
		fprintf(testFile, "\n");
	}

	fprintf(testFile, "Number of records = %ld\n", NumOfRecs);
	fprintf(testFile, "Number of ItemSets = %ld\n", NumOfItemSets);
	fprintf(testFile, "Number of Items = %ld\n", NumOfItems);
	fprintf(testFile, "Average number of ItemSets in a recored = %ld\n",
			NumOfItemSets / NumOfRecs);
	fprintf(testFile, "Average number of Items in an itemset = %ld\n",
			NumOfItems / NumOfItemSets);

	fclose(testFile);
	exit(0);
}
