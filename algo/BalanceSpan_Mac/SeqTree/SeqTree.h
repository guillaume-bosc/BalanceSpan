#if !defined( _SEQTREE_H__ )
#define _SEQTREE_H__

//#define _SORT_RESULTS
#define _FIND_FREQUENT_SEQS
#define _CALC_I_NUM_OF_ITEMS
#define _ANOTHER_CLOSED_APPROACH
//#define _ANOTHER_MAX_APPROACH

#if defined (_ANOTHER_CLOSED_APPROACH)
#include "ClosedTree.h"
#endif

#if !defined( _FIND_FREQUENT_SEQS )
	//#define _DO_NAIVE_APPROACH

	//#define _FIND_MAX_SEQS


	#if !defined( _DO_NAIVE_APPROACH )
		#define _PERFORM_COMMON_PREFIX_CHECKING
	#endif
#else
	//#define _WRITE_FREQUENT_FILE
#endif

#if defined( _FIND_MAX_SEQS )

	#if !defined( _DO_NAIVE_APPROACH )
		#define _USE_MAX_TREE
		#define _USE_STRING_ELEMINATION
	#endif

	#include "MaxSeqTree.h"

#elif !defined( _FIND_FREQUENT_SEQS ) // Find Closed Sequences
	#define _NAIVE_APP 1
	#define _CHECK_COMMON_PREFIX 20

	#if !defined( _FIND_FREQUENT_SEQS ) 
		#if defined( _DO_NAIVE_APPROACH )
			#define _FIND_CLOSED_SEQS _NAIVE_APP
		#else
			#define _FIND_CLOSED_SEQS _CHECK_COMMON_PREFIX
		#endif
	#endif

	#if _FIND_CLOSED_SEQS > _NAIVE_APP
		#define _USE_OUTPUT_BUFFER 
		//#define _NEW_SEQUENCE_LIST 0  // Sort based on support then len (1 dimensional)
		//#define _NEW_SEQUENCE_LIST 10 // Sort based on support then for each supprot have a list sorted based on len (2 dimensional)
		#define _NEW_SEQUENCE_LIST 20 // Use Sequence tree (ClosedTree).
	#endif


	#if _NEW_SEQUENCE_LIST == 10 // Sort based on support then for each supprot have a list sorted based on len (2 dimensional)
		#pragma warning(disable:4786)
		#include <map>
	#endif

	#include "ClosedSeqTree.h"

#endif // defined( _FIND_MAX_SEQS )

#if defined( _PERFORM_COMMON_PREFIX_CHECKING )

class SeqWrap
{
  protected:
		bool StartIsIntra;
		bool FirstItemSet;
		bool CurItemSetIsEmpty;
		int * Current;
		int * Start;
	public:

		SeqWrap();
#if defined( _FIND_MAX_SEQS )
		SeqWrap( int * Strt );
#else
		inline SeqWrap( int * Strt );
#endif
		inline int * GetFirst();
		inline int * GetNext();
		inline int * GetItemSet( int ItemSetNum );
		#if defined( _FIND_MAX_SEQS )
			bool IsEmpty();
		#else
			inline bool IsEmpty();
		#endif // defined( _FIND_MAX_SEQS )

		void Print( FILE * aFile = stdout );

		~SeqWrap();
};

inline int * GetStartPtr( const struct PROJ_DB *pDB, int SeqNum, int SeqIdx = 0 );
inline int * GetEndPtr( int * StartPtr );
inline int * GetNextItem( int * StartPtr );
inline bool SeqIsEmpty( int * StartPtr );

class Prefix : public SeqWrap
{
	private:
		inline void CalcNumOfItemSets();
		inline void TrimPrefix( int * RecStart );
	public:
		int * End;
		int NumOfItemSets;
		int Sup;

		Prefix();
		inline Prefix( int * Strt, int * End );
		Prefix( const struct PROJ_DB * pDB );
		Prefix( const struct mem_map * pDatasetMemMap );
		inline int * GetFirst();
		inline int * GetNext();
		inline int Size();  // Number of frequent elements plus number of itemset seperators, -1s.
		bool IsEmpty();
		void Print( FILE * aFile = stdout );

		~Prefix() {};
};

struct PROJ_DB * MakeProjDB( const struct PROJ_DB* pDB, Prefix * aPrefix );
struct PROJ_DB * MakeProjDBFromOrg(  const struct mem_map * pDB, Prefix * aPrefix, long nCount );

#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )

#if defined( _USE_OUTPUT_BUFFER )
void EmptyBuffer( SequenceList * aList, Sequence * aSeq );
#endif




#endif // !defined( _SEQTREE_H__ )
