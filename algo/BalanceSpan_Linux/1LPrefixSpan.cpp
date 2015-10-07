// PrefixSpan algorithm
// August 2000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Global.h"
#include "ProjDB.h"

/////////////////
// Added by Xifeng
#if defined (_ANOTHER_CLOSED_APPROACH)
TreeNode *root;
TreeNode *currentLevel;
ReverseHashTable *reverseTable; // from NumOfItems to TreeNode*, multimap
int closed_maxFreq[256];
#endif
////////////////
int compteurTest = 0;
void InitApp(const char* filename, const double dSupport,
		const int nItemCount) {

	CreateTimers(3);
	gN_ITEMS = (nItemCount + 1); // because it is zero based.
	gpErrFile = file_open(ERRFILE, "w");
	int nFileSize = InitProjDB(filename);
	gpResultFile = gpStatusFile = stdout;

	//Added by Xifeng
	root = new TreeNode(-1, 0);
	root->ItemsetNumber = 0;
	root->Parent = NULL;
	currentLevel = root;

	reverseTable = new ReverseHashTable;
	for (int i = 0; i < 256; i++)
		closed_maxFreq[i] = 0;

	fprintf(gpStatusFile,
			"1LPrefixSpan: A sequential pattern mining algorithm (pseudo projection).\n");

	fprintf(gpStatusFile,
			"Implemented by Behzad Mortazavi-Asl, in IDBL, SFU\n");
	fprintf(gpStatusFile, "All rights reserved!\n");
	fprintf(gpStatusFile, "Data set (%.3f MB): %s\n",
			nFileSize / (1024.0 * 1024.0), filename);
	fprintf(gpStatusFile, "# of items: %d\n", nItemCount);
	fprintf(gpStatusFile, "Support threshold: %.3f%%\n", dSupport * 100);
	fprintf(gpStatusFile, "-----------------------\n");
}

void CloseApp() {
	DeleteTimers();
	freemem((void**) &gnArrLargeCount);
	freemem((void**) &buf_idx);
	CloseProjDB();
	fclose(gpErrFile);
}

void PrefixSpan(struct PROJ_DB *pDB) {

	int i = 0, j = 0, k = 0, nFreqCount = 0, nFreqCountDual = 0;
	struct PROJ_DB *proj_dbDual = NULL;
	struct PROJ_DB *proj_db = NULL;
	//currentLevel will be modified to next level
	addSequence(pDB, &currentLevel, reverseTable);

	// scan sequence database once, find length-1 sequences

	if (pDB->m_pProjSeq != NULL) {
		proj_db = make_projdb_from_projected_db(pDB, &nFreqCount);
		pDB->SeqId = NULL;
		pDB->isProjected = TRUE;

		// Free useless memory
		for (i = 0; i < nFreqCount; i++) {
			if (proj_db[i].m_nSup < gSUP) {
				n_total_mem -= freePDB(&proj_db[i]);
			} else {
				n_proj_db++;
			}
		}
	}

	if (pDB->dual != NULL && pDB->dual->m_nSup >= gSUP) {

		proj_dbDual = make_projdb_from_projected_db(pDB->dual, &nFreqCountDual);
		pDB->dual->SeqId = NULL;
		pDB->dual->isProjected = TRUE;

		for (i = 0; i < nFreqCountDual; i++) {

			// Free useless memory
			if (proj_dbDual[i].m_nSup < gSUP) {
				n_total_mem -= freePDB(&proj_dbDual[i]);
			} else {
				n_proj_db++;
			}

			// Seek for the dual in the dual projected db and Merge them
			if (proj_dbDual[i].Item % 2 == 0) {
				for (k = 0; k < nFreqCount; k++) {
					if (proj_db[k].Item == proj_dbDual[i].Item + 1
							&& proj_db[k].ItemIsIntra
									== proj_dbDual[i].ItemIsIntra) {
						proj_dbDual[i].dual = &proj_db[k];
						proj_db[k].dual = &proj_dbDual[i];
						break;
					}
				}
			} else {
				for (k = 0; k < nFreqCount; k++) {
					if (proj_db[k].Item == proj_dbDual[i].Item - 1
							&& proj_db[k].ItemIsIntra
									== proj_dbDual[i].ItemIsIntra) {
						proj_dbDual[i].dual = &proj_db[k];
						proj_db[k].dual = &proj_dbDual[i];
						break;
					}
				}
			}
		}
	}

	// Execute the recursion
	if (nFreqCount > 0) {
		for (i = 0; i < nFreqCount; i++) {
			if (proj_db[i].dual != NULL) {
				if (proj_db[i].m_nSup >= gSUP) {
					if (proj_db[i].isProjected == FALSE) {
						PrefixSpan(&(proj_db[i]));
					}
				} else {
					if (proj_db[i].isDone == FALSE
							&& proj_db[i].dual->m_nSup < gSUP) {
						addSequence(&proj_db[i], &currentLevel, reverseTable);
						currentLevel = currentLevel->Parent;
						if (proj_db[i].SeqId != NULL) {
							delete (proj_db[i].SeqId);
							proj_db[i].SeqId = NULL;
						}
						if (proj_db[i].dual->SeqId != NULL) {
							delete (proj_db[i].dual->SeqId);
							proj_db[i].dual->SeqId = NULL;
						}
					}

					//freemem((void**) &(proj_db[i].SeqId));
				}
			} else {
				if (proj_db[i].m_nSup >= gSUP) {
					n_total_mem -= freePDB(&proj_db[i]);
					if (proj_db[i].SeqId != NULL) {
						delete (proj_db[i].SeqId);
						proj_db[i].SeqId = NULL;
					}
				}
			}
		}

	}

	// TODO : Modifier
	if (nFreqCountDual > 0) {
		for (i = 0; i < nFreqCountDual; i++) {

			if (proj_dbDual[i].dual != NULL) {
				if (proj_dbDual[i].m_nSup >= gSUP) {
					if ((proj_dbDual[i].dual->isProjected) == FALSE) {
						PrefixSpan(&(proj_dbDual[i]));
						if (proj_dbDual[i].dual->SeqId != NULL) {
							delete (proj_dbDual[i].dual->SeqId);
							proj_dbDual[i].dual->SeqId = NULL;
						}
					}
				} else {
					if (proj_dbDual[i].SeqId != NULL) {
						delete (proj_dbDual[i].SeqId);
						proj_dbDual[i].SeqId = NULL;
					}
				}
				// pas besoin du ELSE ici
				/*
				 else {
				 if (proj_dbDual[i].isDone == FALSE) {
				 addSequence(&proj_dbDual[i], &currentLevel,
				 reverseTable);
				 currentLevel = currentLevel->Parent;
				 }
				 //freemem((void**) &(proj_dbDual[i].SeqId));
				 }
				 */
			} else {
				if (proj_dbDual[i].m_nSup >= gSUP) {
					if (proj_dbDual[i].SeqId != NULL) {
						delete (proj_dbDual[i].SeqId);
						proj_dbDual[i].SeqId = NULL;
					}
					freePDB(&proj_dbDual[i]);

				}
			}
		}
		n_total_mem -= (nFreqCountDual * sizeof(struct PROJ_DB));
		freemem((void**) &proj_dbDual);
	}

	n_total_mem -= (nFreqCount * sizeof(struct PROJ_DB));
	freemem((void**) &proj_db);

	// Free deque of IDs of the sequences
	/*freemem((void**) &(pDB->SeqId));
	 if (pDB->dual != NULL && pDB->dual->SeqId != NULL) {
	 freemem((void**) &(pDB->dual->SeqId));
	 }*/

	currentLevel = currentLevel->Parent;
}

int main(int argc, char** argv) {

	if (argc != 5) {
		gpErrFile = file_open(ERRFILE, "w");
		if (gpErrFile != NULL) {
			fprintf(gpErrFile,
					"Usage: BalanceSpan <filename> <support> <itemcount> <dictionary_file>\n");
			fclose(gpErrFile);
		}
		printf(
				"Usage: BalanceSpan <filename> <support> <itemcount> <dictionary_file>\n");
		exit(-1);
	}

	// Fill the translation map
	dico = new map<int, string>();
	ifstream fichier(argv[4], ios::in);
	if (fichier) {
		string ligne;
		while (getline(fichier, ligne)) {
			char * lingCh = const_cast<char*>(ligne.c_str());
			char * valeur = strtok(lingCh, "\t\n");
			string identifiant(valeur);

			valeur = strtok(NULL, "\t\n");
			int id = atoi(valeur);

			dico->insert(pair<int, string>(id, identifiant));
		}
		fichier.close();
	}

	InitApp(argv[1], atof(argv[2]), atoi(argv[3]));
	ResetTimer(0);

	int i = 0, j = 0, nFreqCount = 0;
	// scan sequence database once, find length-1 sequences
	struct PROJ_DB *proj_db = make_projdb_from_org_dataset(atof(argv[2]),
			&nFreqCount);

	if (nFreqCount > 0) {
		for (i = 0; i < nFreqCount; i++) {

			if (proj_db[i].dual != NULL) {
				if (proj_db[i].m_nSup < gSUP) {

					if (proj_db[i].isDone == FALSE
							&& proj_db[i].dual->m_nSup < gSUP) {
						addSequence(&proj_db[i], &currentLevel, reverseTable);
						currentLevel = currentLevel->Parent;
					}

					for (j = 0; j < proj_db[i].m_nSup; j++) {

						if (proj_db[i].m_pProjSeq[j].m_nProjCount > 1) {
							n_total_mem -=
									(proj_db[i].m_pProjSeq[j].m_nProjCount
											* sizeof(int*));
							freemem(
									(void**) &(proj_db[i].m_pProjSeq[j].m_ppSeq));
						}
					}
					n_total_mem -= (proj_db[i].m_nMaxSup
							* sizeof(struct PROJ_SEQ));
					freemem((void**) &(proj_db[i].m_pProjSeq));
				} else
					n_proj_db++;
			} else {
				for (j = 0; j < proj_db[i].m_nSup; j++) {

					if (proj_db[i].m_pProjSeq[j].m_nProjCount > 1) {
						n_total_mem -= (proj_db[i].m_pProjSeq[j].m_nProjCount
								* sizeof(int*));
						freemem((void**) &(proj_db[i].m_pProjSeq[j].m_ppSeq));
					}
				}
				n_total_mem -= (proj_db[i].m_nMaxSup * sizeof(struct PROJ_SEQ));
				freemem((void**) &(proj_db[i].m_pProjSeq));
			}
		}

		// TODO : Modifier
		for (i = 0; i < nFreqCount; i++) {
			if (proj_db[i].dual != NULL) {
				if (proj_db[i].m_nSup >= gSUP) {
					if (proj_db[i].isProjected == FALSE) {
						PrefixSpan(&(proj_db[i]));
					}

				}
			}/* else {
			 if (proj_db[i].m_nSup >= gSUP) {
			 for (j = 0; j < proj_db[i].m_nSup; j++) {

			 if (proj_db[i].m_pProjSeq[j].m_nProjCount > 1) {
			 n_total_mem -=
			 (proj_db[i].m_pProjSeq[j].m_nProjCount
			 * sizeof(int*));
			 freemem(
			 (void**) &(proj_db[i].m_pProjSeq[j].m_ppSeq));
			 }
			 }
			 n_total_mem -= (proj_db[i].m_nMaxSup
			 * sizeof(struct PROJ_SEQ));
			 freemem((void**) &(proj_db[i].m_pProjSeq));
			 }
			 }*/

		}
		n_total_mem -= (nFreqCount * sizeof(struct PROJ_DB));
		freemem((void**) &proj_db);
	}

	double TimeDiff = GetTimeDiff(0);
	fprintf(gpResultFile, "%.3f seconds (Total running time)\n", TimeDiff);
	for (i = 1; gnArrLargeCount[i] > 0; i++) {
		fprintf(gpResultFile, "Large %d : %d\n", i, gnArrLargeCount[i]);
		gnArrLargeCount[0] += gnArrLargeCount[i];
	}

	fprintf(gpResultFile, "Total of %d large items.\n", gnArrLargeCount[0]);
	fprintf(gpResultFile, "# of projected datasets: %d\n", n_proj_db);
	fprintf(gpResultFile, "Maximum memory usage: %.3fMB\n",
			double(n_max_mem) / (1024.0 * 1024.0));

	NodeVector::iterator it, endit;
	/*for (it = root->Children->begin(), endit = root->Children->end();
	 it != endit; it++) {
	 closed_maxPruning((*it), root);
	 }*/
	FILE *closed_maxFile = NULL;
	closed_maxFile = file_open("result.txt", "w");

	for (it = root->Children->begin(), endit = root->Children->end();
			it != endit; it++) {
		if ((*it)->Parent == root)
			(*it)->Print("(", "(", closed_maxFile);
	}

	fprintf(closed_maxFile, "\n\nTotal # of TreeNode\t%d\n\n", zzz);

	fprintf(closed_maxFile, "Total # of TreeNode displayed\t%d\n\n",
			nbNodesDisplayed);

	for (i = 0; i < 256; i++)
		if (closed_maxFreq[i] != 0)
			fprintf(closed_maxFile, "Closed/Max\t%d\t%d\n", i,
					closed_maxFreq[i]);

	fprintf(closed_maxFile, "\nTotal of large items\t%d\n\n",
			gnArrLargeCount[0]);
	fprintf(closed_maxFile, "# of projected datasets\t%d\n\n", n_proj_db);
	fprintf(closed_maxFile, "Maximum memory usage\t%.6fMB\n\n",
			double(n_max_mem) / (1024.0 * 1024.0));

	fprintf(closed_maxFile, "Total running time (seconds)\t%.3f\n\n", TimeDiff);
	fprintf(closed_maxFile,
			"Total running time after sort, and output (second)\t%.3f\n",
			GetTimeDiff(0));
	fclose(closed_maxFile);

	/////////////////
	fprintf(gpResultFile,
			"%.3f seconds (Total running time after sort, and output.)\n",
			GetTimeDiff(0));

	CloseApp();
	return 0;
}
