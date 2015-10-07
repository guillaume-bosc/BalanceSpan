#include <assert.h>
#include "../Global.h"
#include "../ProjDB.h"

#include <iostream>
#if defined(_ANOTHER_CLOSED_APPROACH)
#include "ClosedTree.h"

ItemSet supSet;
ItemSet subSet;

int zzz = 0;
int nbNodesDisplayed = 0;

bool TreeNodeLess(TreeNode * a, TreeNode * b) {
	if ((*a).ItemIsIntra == (*b).ItemIsIntra)
		return (*a).Item < (*b).Item;
	else if ((*a).ItemIsIntra && !(*b).ItemIsIntra)
		return true;
	else
		return false;
}

int TreeNodeCompare(TreeNode ** a, TreeNode ** b) {
	if ((*(*a)).ItemIsIntra == (*(*b)).ItemIsIntra)
		if ((*(*a)).Item == (*(*b)).Item)
			return 0;
		else if ((*(*a)).Item < (*(*b)).Item)
			return -1;
		else
			return 1;
	else if ((*(*a)).ItemIsIntra && !(*(*b)).ItemIsIntra)
		return -1;
	else
		return 1;
}

inline TreeNode * GetLastItemSet(TreeNode * treeNode, ItemSet *itemSet) {
	int *itemArray = itemSet->ItemArray;
	int i = 0;

	itemSet->reset();
	do {
		itemArray[i++] = treeNode->Item;
		if (!treeNode->ItemIsIntra)
			break;
		else
			treeNode = treeNode->Parent;
	} while (treeNode->Items > 0);

	itemSet->Count = i;
	return (treeNode->Parent);
}

inline int isContained_DBSizeEqual(struct PROJ_DB *pDB, TreeNode *currentLevel,
		TreeNode *candidate) {

	int myItem = pDB->Item;
	bool myItemIsIntra = pDB->ItemIsIntra;
	int dir;
	bool isContained;

	TreeNode *supNode;
	TreeNode *subNode;

	if ((myItem != candidate->Item)
			|| (candidate->Items == (currentLevel->Items + 1)))
		return 0;

	if (candidate->Items > (currentLevel->Items + 1))
		dir = 1;
	else
		dir = -1;

	if (candidate->ItemIsIntra) {
		if (myItemIsIntra) {
			candidate = candidate->Parent;
		} else {
			//in this case, supNode must be candidate, subNode must be currentLevel
			while (candidate->ItemIsIntra && candidate->Items > 0)
				candidate = candidate->Parent;

			candidate = candidate->Parent;
			if ((candidate->Items < currentLevel->Items)
					|| (candidate->ItemsetNumber < currentLevel->ItemsetNumber))
				return 0;

		}
	} else {
		if (myItemIsIntra) {
			//in this case, supNode must be currentLevel, subNode must be candidate
			while (currentLevel->ItemIsIntra && currentLevel->Items > 0)
				currentLevel = currentLevel->Parent;

			currentLevel = currentLevel->Parent;
			if ((candidate->Items > currentLevel->Items)
					|| (candidate->ItemsetNumber > currentLevel->ItemsetNumber))
				return 0;

		} else {
			candidate = candidate->Parent;
		}
	}

	if (currentLevel->Items == 0) {
		return 1;
	}

	if (candidate->Items == 0) {
		return -1;
	}

	if (dir == 1) {
		supNode = candidate;
		subNode = currentLevel;
	} else {
		supNode = currentLevel;
		subNode = candidate;
	}

	isContained = false;
	supNode = GetLastItemSet(supNode, &supSet);
	subNode = GetLastItemSet(subNode, &subSet);

	while (1) {
		if (subSet.IsSubsetOf(&supSet)) {
			if (subNode->Items == 0) {
				isContained = true;
				break;
			}
			if ((supNode->Items >= subNode->Items)
					&& (supNode->ItemsetNumber >= subNode->ItemsetNumber)) {
				supNode = GetLastItemSet(supNode, &supSet);
				subNode = GetLastItemSet(subNode, &subSet);
			} else
				break;
		} else {
			if (supNode->Items == 0) // unnecessary to check any more.
				break;

			if ((supNode->Items >= subNode->Items)
					&& (supNode->ItemsetNumber >= subNode->ItemsetNumber)) {
				supNode = GetLastItemSet(supNode, &supSet);
			} else
				break;
		}

	}
	if (isContained)
		return dir;
	else
		return 0;

}

void updateNodeInfo(TreeNode *treeNode, int extraItems,
		int extraItemsetNumber) {
	NodeVector::iterator it, endit;

	treeNode->Items += extraItems;
	treeNode->ItemsetNumber += extraItemsetNumber;
	for (it = treeNode->Children->begin(), endit = treeNode->Children->end();
			it != endit; it++) {
		if ((*it)->Parent == treeNode)
			updateNodeInfo((*it), extraItems, extraItemsetNumber);
	}
}

void updateOldNodeInfo(TreeNode* parent, TreeNode *pNode, TreeNode *qNode) {
	TreeNode **Res;
	Res = (TreeNode **) bsearch(&pNode, &(*(parent->Children))[0],
			(*(parent->Children)).size(), sizeof(TreeNode *),
			(int (*)(const void*, const void*))TreeNodeCompare);*
	Res = qNode;
}

void checkNodeInfo(TreeNode *treeNode) {
	int items;
	int itemset_number;
	TreeNode *parent = treeNode->Parent;
	NodeVector::iterator it, endit;

	if (treeNode->ItemIsIntra) {
		itemset_number = parent->ItemsetNumber;
		items = parent->Items + 1;
	} else {
		itemset_number = parent->ItemsetNumber + 1;
		items = parent->Items + 1;
	}

	if ((items != treeNode->Items)
			|| (itemset_number != treeNode->ItemsetNumber))
		printf("Node Info: Items or ItemsetNumber Error");

	for (it = treeNode->Children->begin(), endit = treeNode->Children->end();
			it != endit; it++) {
		if ((*it)->Parent == treeNode)
			checkNodeInfo((*it));
	}

}

int addSequence(struct PROJ_DB *pDB, TreeNode **currentLevelp,
		ReverseHashTable *reverseTable) {
	TreeNode *pNode;
	TreeNode *currentLevel = *currentLevelp;

	// TODO : A decommenter si soucis
	/*
	 ReverseMap pMap;
	 ReverseHashTable::iterator it, endit;

	 int ret, myNumOfItems;

	 myNumOfItems = pDB->NumOfItems + pDB->m_nMaxSup;

	 pMap = (*reverseTable).equal_range(myNumOfItems);
	 for (it = pMap.first, endit = pMap.second; it != endit; it++) {
	 pNode = (*it).second;
	 //cout << " pNode->Support " << pNode->Support << endl;
	 //cout << " pNode->Item " << pNode->Item << endl;
	 if (pNode->Support != pDB->m_nMaxSup)
	 continue;
	 ret = isContained_DBSizeEqual(pDB, currentLevel, pNode);

	 }
	 */

	zzz++;
	//pNode = new TreeNode();
	pNode = currentLevel->AddChildWithoutChecking(pDB);
	(*currentLevelp) = pNode;

	//pNode->SetProjDBSize(myNumOfItems);
	//reverseTable->insert(ReverseHashTable::value_type(myNumOfItems, pNode));

	return INEQUAL_PROJECT_DB_SIZE;

}

void closed_maxChecking(TreeNode *pNode, TreeNode *pNode_parent,
		TreeNode *qNode) {
	NodeVector::iterator it, endit;

	if (pNode == NULL)
		return;

	if (pNode->Parent != pNode_parent)
		return;

	if (pNode->Support == qNode->Support)

		for (it = qNode->Children->begin(), endit = qNode->Children->end();
				it != endit; it++)
			closed_maxChecking(
					pNode->FindChild((*it)->Item, (*it)->ItemIsIntra), pNode,
					(*it));

}

void closed_maxPruning(TreeNode *treeNode, TreeNode *parent) {
	NodeVector::iterator it, endit, ti;
	bool myItemIsIntra = treeNode->ItemIsIntra;
	TreeNode *pNode;

	if (parent->Support == treeNode->Support) {
		parent->closed = false;
	}

	for (it = treeNode->Children->begin(), endit = treeNode->Children->end();
			it != endit; it++) {
		if ((*it)->Parent == treeNode)
			closed_maxPruning((*it), treeNode);
	}
	//check  subpattern closed

	for (it = treeNode->Children->begin(), endit = treeNode->Children->end();
			it != endit; it++) {
		if (!((*it)->ItemIsIntra && myItemIsIntra))
			pNode = parent->FindChild((*it)->Item, false);
		else
			pNode = parent->FindChild((*it)->Item, true);

		closed_maxChecking(pNode, parent, (*it));
	}

}

inline bool ItemSet::IsSubsetOf(ItemSet * anItemSet) {
	int j;
	int OtherCount = anItemSet->Count;

	if (Count > OtherCount)
		return false;

	j = 0;
	for (int i = 0; i < Count; i++) {
		while (ItemArray[i] != anItemSet->ItemArray[j]) {
			j++;
			if ((OtherCount - j) < (Count - i))
				return false;
		}
	}

	return true;
}

LevelNode::LevelNode() {
	reverseTable = NULL;
}

inline bool LevelNode::isEmpty() {
	if ((reverseTable == NULL) || (*reverseTable).size() == 0)
		return true;
	else
		return false;
}

inline void LevelNode::addCandidate(TreeNode *treeNode) {
	NodeVector::iterator it, endit;
	if (reverseTable == NULL)
		reverseTable = new ReverseNodeHashTable;
	for (it = treeNode->Children->begin(), endit = treeNode->Children->end();
			it != endit; it++) {
		(*reverseTable).insert(
				ReverseNodeHashTable::value_type((*it)->Item, (*it)));
	}
}

inline ReverseNodeMap LevelNode::findCandidate(int Item) {
	return ((*reverseTable).equal_range(Item));
}

LevelNode::~LevelNode() {
	if (reverseTable != NULL)
		delete reverseTable;
}

TreeNode::TreeNode(int anItem, bool IsIntra, int Sup, TreeNode * aParent) {

	Children = new NodeVector();

	Parent = aParent;
	ItemsetNumber = 0;
	Items = 0;
	Item = anItem;
	ItemIsIntra = IsIntra;
	Support = Sup;
	closed = true;
	max = true;
}

TreeNode::TreeNode(TreeNode *treeNode) {

	Children = treeNode->Children;

	Parent = treeNode->Parent;
	ItemsetNumber = treeNode->ItemsetNumber;
	Items = treeNode->Items;
	Item = treeNode->Item;
	ItemIsIntra = treeNode->ItemIsIntra;
	Support = treeNode->Support;
	closed = true;
	max = true;
}

inline TreeNode * TreeNode::FindChild(int anItem, bool Intra) {
	TreeNode ** Res;
	TreeNode * tmp = new TreeNode(anItem, Intra);

	Res = (TreeNode **) bsearch(&tmp, &(*Children)[0], (*Children).size(),
			sizeof(TreeNode *),
			(int (*)(const void*, const void*))TreeNodeCompare);

	delete tmp;

	if (Res)
		return (*Res);
	else
		return NULL;
}

inline void TreeNode::DelChild(int anItem, bool Intra,
		NodeVector::iterator it) {
	TreeNode *theNode = NULL;

	theNode = FindChild(anItem, Intra);
	if (theNode != NULL) {
		(*Children).erase(it);
		delete theNode;
	}
}

inline void TreeNode::DelChild(TreeNode * Child, NodeVector::iterator it) {

	if (Children == NULL)
		return;
	(*Children).erase(it);
}

TreeNode * TreeNode::AddChild(int anItem, bool Intra, int Sup) {
	TreeNode *Result = NULL;
	TreeNode *Child = NULL;

	Result = FindChild(anItem, Intra);
	if (Result == NULL) {
		Child = new TreeNode(anItem, Intra, Sup);
		Result = Child;
		(*Children).push_back(Child);
		// To keep the children vector sorted.
		inplace_merge((*Children).begin(), (*Children).end() - 1,
				(*Children).end(), TreeNodeLess);
		Child->Parent = this;
		if (Child->ItemIsIntra) {
			Child->ItemsetNumber = ItemsetNumber;
			Child->Items = Items + 1;
		} else {
			Child->ItemsetNumber = ItemsetNumber + 1;
			Child->Items = Items + 1;
		}
	} else {
		if (Result->Support < Sup) {
			printf("Item: %d %d %d\n", anItem, Result->Support, Sup);
			printf("ParentItem: %d %d\n", Item, Support);
			Result->Support = Sup;
		}
	}
	return Result;
}

//TODO : Si dual ou orignal = null !
inline TreeNode *TreeNode::AddChildWithoutChecking(struct PROJ_DB *pDB) {
	TreeNode *Child;
	deque<int> intersection;

	pDB->isDone = TRUE;
	if (pDB->dual != NULL) {
		pDB->dual->isDone = TRUE;
		set_intersection(pDB->SeqId->begin(), pDB->SeqId->end(),
				pDB->dual->SeqId->begin(), pDB->dual->SeqId->end(),
				insert_iterator<deque<int> >(intersection,
						intersection.begin()));
	}

	if (pDB->status == ORIGINAL) {
		Child = new TreeNode(pDB->Item, pDB->ItemIsIntra, pDB->m_nMaxSup);

		Child->SupportIncomp = pDB->m_nMaxSup - intersection.size();

		if (pDB->dual != NULL) {
			Child->ItemDual = pDB->dual->Item;
			Child->SupportDual = pDB->dual->m_nMaxSup;
			Child->SupportDualIncomp = pDB->dual->m_nMaxSup
					- intersection.size();
			Child->balance =
					(double) ((double) Child->SupportIncomp
							/ (double) (Child->SupportIncomp
									+ Child->SupportDualIncomp));
		} else {
			Child->ItemDual = -1;
			Child->SupportDual = 0;
			Child->SupportDualIncomp = 0;
			Child->balance = (double) 1;
		}

	} else {
		if (pDB->dual != NULL) {
			Child = new TreeNode(pDB->dual->Item, pDB->dual->ItemIsIntra,
					pDB->dual->m_nMaxSup);
			Child->SupportIncomp = pDB->dual->m_nMaxSup - intersection.size();
		} else {
			Child = new TreeNode(-1, pDB->ItemIsIntra, 0);
			Child->SupportIncomp = 0;
			Child->balance = (double) 1;
		}

		Child->ItemDual = pDB->Item;
		Child->SupportDual = pDB->m_nMaxSup;
		Child->SupportDualIncomp = pDB->m_nMaxSup - intersection.size();
		Child->balance = (double) ((double) Child->SupportDualIncomp
				/ (double) (Child->SupportIncomp + Child->SupportDualIncomp));

	}

	(*Children).push_back(Child);
	// To keep the children vector sorted.
	inplace_merge((*Children).begin(), (*Children).end() - 1, (*Children).end(),
			TreeNodeLess);
	Child->Parent = this;
	if (Child->ItemIsIntra) {
		Child->ItemsetNumber = ItemsetNumber;
		Child->Items = Items + 1;
	} else {
		Child->ItemsetNumber = ItemsetNumber + 1;
		Child->Items = Items + 1;
	}
	return Child;
}

TreeNode * TreeNode::AddChild(TreeNode * Child) {
	TreeNode * Result = NULL;

	Result = FindChild(Child->Item, Child->ItemIsIntra);
	if (Result == NULL) {
		Result = Child;
		(*Children).push_back(Child);
		// To keep the children vector sorted.
		//inplace_merge( (*Children).begin(), (*Children).end()-1, (*Children).end(), TreeNodeLess );
		Child->Parent = this;
		if (Child->ItemIsIntra) {
			Child->ItemsetNumber = ItemsetNumber;
			Child->Items = Items + 1;
		} else {
			Child->ItemsetNumber = ItemsetNumber + 1;
			Child->Items = Items + 1;
		}
	} else {
		if (Child->Support > Result->Support)
			Result->Support = Child->Support;
		delete Child;

	}
	return Result;
}

inline void TreeNode::SetProjDBSize(long size) {
	NumOfItems = size;
}

bool TreeNode::isRoot() {
	if (Parent == NULL)
		return true;
	else
		return false;
}
inline int TreeNode::NumOfChildren() {
	if (Children == NULL)
		return 0;
	else
		return (*Children).size();
}

inline int TreeNode::MaxChildSupport() {
	int maxChildSup = 0;
	NodeVector::iterator it;

	if (Children == NULL)
		return 0;

	for (it = (*Children).begin(); it != (*Children).end(); it++)
		if (((*it))->Support > maxChildSup)
			maxChildSup = (*it)->Support;

	return maxChildSup;
}

bool TreeNode::LastItemOfSequence() {
	if (Children == NULL)
		return true;

	if (MaxChildSupport() < Support)
		return true;

	return false;
}

void TreeNode::Print(char *PrefixString, char *PrefixStringDual, FILE * aFile) {
	NodeVector::iterator it;
	char NewPrefixString[256];
	char NewPrefixStringDual[256];
	char * identifiant = const_cast<char*>((*dico)[Item].c_str());
	char * identifiantDual = const_cast<char*>((*dico)[ItemDual].c_str());

	if (ItemIsIntra) {
		if (Item != -1)
			sprintf(NewPrefixString, "%s,%s", PrefixString, identifiant);
		if (ItemDual != -1)
			sprintf(NewPrefixStringDual, "%s,%s", PrefixStringDual,
					identifiantDual);
	} else {
		if (Item != -1)
			sprintf(NewPrefixString, "%s)(%s", PrefixString, identifiant);
		if (ItemDual != -1)
			sprintf(NewPrefixStringDual, "%s)(%s", PrefixStringDual,
					identifiantDual);
	}

	if (Item != -1) {
		fprintf(aFile,
				"<%s)>\tBalance = %f\tSupportTot = %d\tSupportInters = %d\tSupportDualTot = %d\tSupportDualInters = %d\n",
				NewPrefixString + 2, balance, Support, SupportIncomp,
				SupportDual, SupportDualIncomp);

		nbNodesDisplayed++;

	} else if (ItemDual != -1) {
		fprintf(aFile,
				"<%s)>\tBalance = %f\tSupportTot = %d\tSupportInters = %d\tSupportDualTot = %d\tSupportDualInters = %d\n",
				NewPrefixStringDual + 2, balance, SupportDual,
				SupportDualIncomp, Support, SupportIncomp);

		nbNodesDisplayed++;

	}
	if (Children != NULL && NumOfChildren() > 0) {
		for (it = (*Children).begin(); it != (*Children).end(); it++) {
			if ((*it)->Parent == this) // if not equal, its subtree must be totally absorbed
				(*it)->Print(NewPrefixString, NewPrefixStringDual, aFile);
		}
	}

	closed_maxFreq[Items]++;

}

TreeNode::~TreeNode() {
	NodeVector::iterator it;

	if (Children)
		for (it = (*Children).begin(); it != (*Children).end(); it++)
			delete *it;
	delete Children;
}

#endif // !defined( _FIND_MAX_SEQS ) && !defined( _FIND_FREQUENT_SEQS )
