#if !defined( _CLOSED_TREE_H__ )
#define _CLOSED_TREE_H__

#include <stdio.h> 
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

//#ifdef _MSC_VER //// modif mehdi
#include <unordered_map> //// modif mehdi
//#else //// modif mehdi
//#include <ext/unordered_map> //// modif mehdi
//using namespace __gnu_cxx; //// modif mehdi
//#endif //// modif mehdi

#define EQUAL_PROJECT_DB_SIZE 0
#define INEQUAL_PROJECT_DB_SIZE 1

using namespace std;

class TreeNode;
class LevelNode;

typedef unordered_map<int, TreeNode *, hash<int>, equal_to<int> > NodeHashTable; ///// modif mehdi
typedef unordered_multimap<int, TreeNode *, hash<int>, equal_to<int> > ReverseNodeHashTable; /// modif mehdi
typedef unordered_multimap<int, TreeNode *, hash<int>, equal_to<int> > ReverseHashTable; /// modif mehdi
typedef pair<ReverseNodeHashTable::iterator, ReverseNodeHashTable::iterator> ReverseMap;

typedef unordered_multimap<int, TreeNode *, hash<int>, equal_to<int> > ReverseNodeHashTable;
typedef pair<ReverseNodeHashTable::iterator, ReverseNodeHashTable::iterator> ReverseNodeMap;


typedef vector<TreeNode *> NodeVector;
typedef vector<NodeVector *> NodeVectorTable;
typedef vector<LevelNode *> LevelVector;
 
class ItemSet
{
	public:
		int ItemArray[64];
		int Count;

		ItemSet() { Count=0; };
		inline bool IsSubsetOf( ItemSet * anItemSet );
		int size() {  return Count; };
		inline void	reset() { Count=0; };

		~ItemSet() { };
};

class LevelNode
{
	public:
		ReverseNodeHashTable *reverseTable;
		LevelNode();
		inline bool isEmpty();
		inline void addCandidate(TreeNode *treeNode);
		inline ReverseNodeMap findCandidate(int Item);
		~LevelNode();
};

class TreeNode
{
	public:
		int		Item;
		bool	ItemIsIntra;
		int		ItemsetNumber;
		int		Items;
		int		Support;
		int		SupportIncomp;
		long	NumOfItems;

		// Take into account the dual of the sequence
		int		ItemDual;
		int		SupportDual;
		int		SupportDualIncomp;
		bool 	isSwitched;
		double	balance;

		bool	closed;
		bool	max;
		
		NodeVector		*Children;
		TreeNode		*Parent;

		TreeNode( int anItem = -1, bool ItemIsIntra = false, int Sup = 0, TreeNode * aParent = NULL );
		TreeNode( TreeNode *treeNode);

		inline void		DelChild( TreeNode * Child, NodeVector::iterator it);
		inline void		DelChild( int anItem, bool Intra, NodeVector::iterator it);
		inline TreeNode *FindChild( int anItem, bool Intra );

		TreeNode * AddChild( int anItem, bool Intra, int Sup =0);
		TreeNode * AddChild(TreeNode * Child );
		inline TreeNode *AddChildWithoutChecking(struct PROJ_DB *pDB);
		inline void	SetProjDBSize(long size);
		inline int NumOfChildren();
		inline int MaxChildSupport();
		bool LastItemOfSequence();
		bool isRoot();
		void Print( char* PrefixString = "", char *PrefixStringDual = "", FILE * aFile = stdout );

		~TreeNode();
};

typedef vector<TreeNode *> TreeNodeList;
extern int addSequence(struct PROJ_DB *pDB, TreeNode **currentLevelp, ReverseHashTable *reverseTable);
extern void closed_maxPruning(TreeNode *treeNode, TreeNode *parent);

extern int closed_maxFreq[256];
extern int zzz;
extern int nbNodesDisplayed;

#endif // !defined( _CLOSED_TREE_H__ )
