
#include <assert.h>
#include "../Global.h"
#include "../ProjDB.h"
#include "../MemMap.h"

#if defined( _PERFORM_COMMON_PREFIX_CHECKING )


// Returns a pointer to the start of the SeqNum th sequence of the projected DB.
//inline int * GetStartPtr( const struct PROJ_DB *pDB, int SeqNum, int SeqIdx = 0 )
inline int * GetStartPtr( const struct PROJ_DB *pDB, int SeqNum, int SeqIdx )
{
	int * intPtr;

	if( SeqIdx >= (*pDB).m_pProjSeq[SeqNum].m_nProjCount )
		assert( false );

#ifdef DISK_BASED
	if( (*pDB).m_pProjSeq[SeqNum].m_nProjCount == 1 )
		intPtr = (int*) (*pDB).m_pProjSeq[SeqNum].m_ppSeq;
	else
		intPtr = (int*) (*pDB).m_pProjSeq[SeqNum].m_ppSeq[SeqIdx];
#else
	intPtr = (int*) (*pDB).m_pProjSeq[SeqNum].m_ppSeq[SeqIdx];
#endif

	return intPtr;
}

// Returns a pointer to the End of the Sequence that starts at StartPtr.
inline int * GetEndPtr( int * StartPtr )
{
	int * TmpPtr;

	TmpPtr = StartPtr;
	while( *TmpPtr != -2 )
		TmpPtr++;
	return TmpPtr;
}

// Returns a pointer to the first frequent item pointed by StartPtr, or after it.
// It returns a pointer to -1 for end of itemset, and -2 for end of the sequence.
inline int * GetNextItem( int * StartPtr )
{
#ifdef DISK_BASED
	int * DataPtr = StartPtr;
	bool InFreqItemSet = false;

	while( true )
	{
		//if( !(*DataPtr == -1 && InFreqItemSet) && *DataPtr < 0 )
		if( *DataPtr < 0 )
			return DataPtr;

		if( (inter[*DataPtr]).count >= gSUP )
			return DataPtr;

		InFreqItemSet = true;
		DataPtr++;
	}
#else
	return StartPtr;
#endif
}

// Returns true is the sequence pointed to by StartPtr has no frequent items.
inline bool SeqIsEmpty( int * StartPtr )
{
	int * Ptr;
	
	Ptr = GetNextItem( StartPtr );
	while( *Ptr == -1 )
	{
		Ptr = GetNextItem( ++Ptr );
	}

	if( *Ptr == -2 )
		return true;
	else
		return false;
}

//************************
//************************

SeqWrap::SeqWrap() 
{ 
	Start = NULL; 
	Current = Start; 
	FirstItemSet = true; 
	CurItemSetIsEmpty = true;
	StartIsIntra = true;
}

#if defined( _FIND_MAX_SEQS )
SeqWrap::SeqWrap( int * Strt ) 
#else
inline SeqWrap::SeqWrap( int * Strt ) 
#endif
{ 
	FirstItemSet = true; 
	Start = Strt;
	StartIsIntra = true;

	while( true )
	{
		if( *Start < 0 )
		{
			StartIsIntra = false;
			break;
		}

		if( (intra[*Start]).count >= gSUP )
			break;

		Start++;
	}
	FirstItemSet = StartIsIntra;
	Current = Start; 
}

inline int * SeqWrap::GetFirst() 
{ 
	Current = Start;
	FirstItemSet = StartIsIntra;
	CurItemSetIsEmpty = true;
	return Current;
}

inline int * SeqWrap::GetNext()
{

	if( Current == NULL || *Current == -2 )
		return Current;

	if( *Current == -1 )
	{ 
		FirstItemSet = false;
		CurItemSetIsEmpty = true;
	}

	Current++;

	while( true )
	{
		if( *Current == -2 )
			return Current;

		if( *Current != -1 ) 
		{
			if( ( !FirstItemSet && (inter[*Current]).count >= gSUP ) ||
				( FirstItemSet && (intra[*Current]).count >= gSUP ) )
			{
				CurItemSetIsEmpty = false;
				return Current;
			}
		} else { //*Current==-1
			if( FirstItemSet )
			{
				FirstItemSet = false;
				CurItemSetIsEmpty = true;
				return Current;
			} else {
				if( !CurItemSetIsEmpty )
				{
					CurItemSetIsEmpty = true;
					return Current;
				}
				CurItemSetIsEmpty = true;
			}
		}

		Current++;
	}
}

inline int * SeqWrap::GetItemSet( int ItemSetNum )
{
	int tmp = 0;
	
	Current = GetFirst();
	if( *Current == -1 )
		Current = GetNext();

	while( *Current != -2 )
	{
		if( *Current == -1 )
		{
			tmp++;
			if( tmp == ItemSetNum )
				break;
		}
		Current = GetNext();
	}

	return Current;
	//return GetNext();
}

void SeqWrap::Print( FILE * aFile )
{
	int * TmpPtr = NULL;
	int * TmpPtr2 = NULL;

	fprintf( aFile, " <" );


	TmpPtr = GetFirst();

	if( TmpPtr!=NULL )
		fprintf( aFile, " (" );

	while( TmpPtr != NULL && *TmpPtr != -2 )
	{
		if( *TmpPtr == -1 )
		{
			TmpPtr2 = GetNext();
			if( *TmpPtr2 == -2 )
				fprintf( aFile, ") " );
			else
				fprintf( aFile, ")  (" );
			TmpPtr = TmpPtr2;
		} else {
			fprintf( aFile, " %d ", *TmpPtr );
			TmpPtr = GetNext();
		}

	}

	fprintf( aFile, "> " );

	fprintf( aFile, "  Support = %d\n", 0 );
}

#if defined( _FIND_MAX_SEQS )
bool SeqWrap::IsEmpty()
#else
inline bool SeqWrap::IsEmpty()
#endif // defined( _FIND_MAX_SEQS )
{
	
	Current = GetFirst();
	while( *Current == -1 )
	{
		Current = GetNext();
	}

	if( *Current == -2 )
		return true;
	else
		return false;
}

SeqWrap::~SeqWrap()
{
}

//************************
//************************

Prefix::Prefix() : SeqWrap()
{ 
	End = NULL;
	NumOfItemSets = 0;
	Sup = 0;
}

inline Prefix::Prefix( int * Strt, int * End ) : SeqWrap( Strt )
{ 
	NumOfItemSets = 0;
	(*this).End = End;
	if( Start == End )	
	{
		Start = NULL;
		(*this).End = NULL;
		NumOfItemSets = 0;
	} else {
		CalcNumOfItemSets();
	}
	Sup = 0;
}

Prefix::Prefix( const struct PROJ_DB * pDB ) : SeqWrap()
{
//#define GetDBPrefix_PRINT_DEBUG_INFO

	int i = 0;
	int j = 0;
	int JMax = 1;
	bool Inter;
	SeqWrap * aSeqWrap = NULL;
	bool PrefixIsEmpty = true;

#ifdef GetDBPrefix_PRINT_DEBUG_INFO

	Sequence * aSeq = NULL;
	Sequence * tmpSeq = NULL;
	static int Cnt = 0;

	tmpSeq = new Sequence( pDB );
	printf( "  Original Prefix %dth Record ===> ", Cnt++ );
	(*tmpSeq).Print();
	delete tmpSeq;
#endif
	
	End = NULL;
	NumOfItemSets = 0;
	//Sup = 1;
	Sup = 0; 
	if( (*pDB).m_nSup > 0 )
	{
		// Find first none empty sequence.
		for( i=0; i<(*pDB).m_nSup; i++ )
		{
			for( j=0; j < (*pDB).m_pProjSeq[i].m_nProjCount; j++ )
			{
				aSeqWrap = new SeqWrap( GetStartPtr( pDB, i, j ) );

				if( !(*aSeqWrap).IsEmpty() )
				{
					Start = (*aSeqWrap).GetFirst();
					End = GetEndPtr( Start ) - 1;
					//TrimPrefix( Start ); //Trim it with itself.
					PrefixIsEmpty = false;
					#ifdef GetDBPrefix_PRINT_DEBUG_INFO
						aSeq = new Sequence( Start, End-Start, 0, true );
						printf( "  First Prefix %dth Record ===> ", i+1 );
						if( aSeq ) (*aSeq).Print();
						printf( "    Real First Start=%d End=%d Size=%d ===> ", Start, End, End-Start );
						Print();
					#endif

					delete aSeqWrap;
					break;
				}
				delete aSeqWrap;
			}
			if( Start != End )
				break;
		}
	}

	if( Start != End )
	{
		JMax = 1;
		if( *(Start) == -1 )
			Inter = true;
		else
			Inter = false;

		//for( i++; i<(*pDB).m_nSup; i++ ) // For every seq in DB
		for( i; i<(*pDB).m_nSup; i++ ) // For every seq in DB
		{
			if( !Inter )
				JMax = (*pDB).m_pProjSeq[i].m_nProjCount;

			//JMax = 1;
			for( j=0; j < JMax; j++ )
			{
				aSeqWrap = new SeqWrap( GetStartPtr( pDB, i, j ) );

#ifdef GetDBPrefix_PRINT_DEBUG_INFO
				tmpSeq = new Sequence( (*aSeqWrap).GetFirst(), true );
				printf( "  %dth Record ===> ", i+1 );
				(*tmpSeq).Print();
				delete tmpSeq;
#endif


				if( !(*aSeqWrap).IsEmpty() )
				{
					TrimPrefix( (*aSeqWrap).GetFirst() );
					Sup++;
#ifdef GetDBPrefix_PRINT_DEBUG_INFO
					if( Start != End )
					{
						aSeq = new Sequence( Start, End-Start, 0, true );
						printf( "     ++++++++++++++++ Remaining prefix is  " );
						(*aSeq).Print();
						printf( "        +++++Real Start=%d End=%d Size=%d prefix is ", Start, End, End-Start );
						Print();
					}
#endif
				}
				delete aSeqWrap;

				PrefixIsEmpty = IsEmpty();
				if( PrefixIsEmpty )
					break;
			}				
			if( PrefixIsEmpty )
				break;
		}
	}

	//if( Start == End )	
	if( PrefixIsEmpty )	
	{
		Start = NULL;
		End = NULL;
		NumOfItemSets = 0;
		Sup = 0;
	} else {
		CalcNumOfItemSets();
	}

}

Prefix::Prefix( const struct mem_map * pDatasetMemMap ) : SeqWrap()
{
	bool PrefixIsEmpty = true;

	int * RecStart;
  int * LastAddr = (int*) GetLastAddrOfMap( pDatasetMemMap );
	
	NumOfItemSets = 0;
	Sup = 1; 

	Start = (int*) GetStartOfMap( pDatasetMemMap );
	Start = GetNextItem( Start ); // Get first frequent item.
	while( *Start < 0 && Start < LastAddr )
		Start = GetNextItem( Start+1 ); // Get first frequent item.

	End = Start;
	// Find first none empty sequence.
	while( End < LastAddr )
	{
		End = GetEndPtr(Start) - 1;

		if( Start != End )
			break;
		else
			Start = GetNextItem( End + 2 );
	}

	RecStart = GetNextItem( End + 2 );
	while( *RecStart < 0 && RecStart < LastAddr )
		RecStart = GetNextItem( RecStart + 1 );

	if( Start != End )
	{
		while( RecStart < LastAddr )
		{
			TrimPrefix( RecStart );

			PrefixIsEmpty = IsEmpty();
			if( PrefixIsEmpty )
				break;

			Sup++;
			RecStart = GetEndPtr(RecStart) + 1;
			while( *RecStart < 0 && RecStart < LastAddr )
				RecStart = GetNextItem( RecStart + 1 );
		}
	}

	if( PrefixIsEmpty )	
	{
		Start = NULL;
		End = NULL;
		NumOfItemSets = 0;
		Sup = 0;
	} else {
		CalcNumOfItemSets();
	}
}

// Returns number of itemsets that have at least one frequent item.
// It starts from StartPtr to EndPtr, if EndPtr is NULL it will search till end of the sequence.
inline void Prefix::CalcNumOfItemSets()
{
	//bool LastWasNegOne = false;
	int Result = 0;

	if( Start==End )
	{
		NumOfItemSets = 0;
		return;
	}

	Current = GetFirst();

	if( End==NULL )
	{
		while( *Current!=-2 )
		{
			//LastWasNegOne = ( *Current==-1 );
			Current = GetNext();
			if( *Current==-1 )
				Result++;
		}
	} else {
		while( Current<End )
		{
			//LastWasNegOne = ( *Current==-1 );
			Current = GetNext();
			if( *Current==-1 )
				Result++;
		}
	}

	//if( LastWasNegOne )
	//	Result--;

	NumOfItemSets = Result;
}

// Number of frequent elements plus number of itemset seperators, -1s, but last one.
inline int Prefix::Size()
{
	//bool LastWasNegOne = false;
	int tmp = 0;

	Current = GetFirst();

	while( Current < End && *Current != -2 )
	{
		//LastWasNegOne = ( *Current==-1 );
		tmp++;
		Current = GetNext();
	}

	//if( LastWasNegOne )
	//	tmp--;

	return tmp;
}

inline int * Prefix::GetFirst() 
{ 
	Current = SeqWrap::GetFirst();

	if( End!=NULL && Current > End )
		Current = End;

	return Current; 
}

inline int * Prefix::GetNext() 
{ 
	Current = SeqWrap::GetNext();

	if( End!=NULL && Current > End )
		Current = End;

	return Current;
}

void Prefix::Print( FILE * aFile )
{
//#define _TEST_SIZE

	int * TmpPtr = NULL;
	int * TmpPtr2 = NULL;
#if defined( _TEST_SIZE )
	int SizeCnt = 0;
	int ElmntCnt = 0;
#endif

	fprintf( aFile, " Elmnts=%d  Size=%d <", NumOfItemSets, Size() );

	TmpPtr = GetFirst();

	if( TmpPtr!=NULL )
		fprintf( aFile, " (" );

	while( TmpPtr != NULL )
	{
		if( *TmpPtr == -1 )
		{
			TmpPtr2 = GetNext();
			if( *TmpPtr2 == -2 || TmpPtr2 == End )
			{
				fprintf( aFile, ") " );
				break;
			} else {
				fprintf( aFile, ")  (" );
				#if defined( _TEST_SIZE )
				SizeCnt++;
				ElmntCnt++;
				#endif
			}
			TmpPtr = TmpPtr2;
		} else {
			fprintf( aFile, " %d ", *TmpPtr );
			TmpPtr = GetNext();
			#if defined( _TEST_SIZE )
			SizeCnt++;
			if( ElmntCnt == 0 )
				ElmntCnt = 1;
			#endif
		}

	}

	fprintf( aFile, "> " );

	fprintf( aFile, "  Support = %d\n", Sup );

#if defined( _TEST_SIZE )
	int tmp = 0;
	if( SizeCnt != Size() || ElmntCnt != NumOfItemSets )
	{
		printf( "RTA RTA RTA\n" );
		tmp = Size();
		CalcNumOfItemSets();
		//TrimPrefix( (*this).Start );
		//exit(1);
	}
#endif
}

bool Prefix::IsEmpty()
{

	if( Start == End )
		return true;

	Current = GetFirst();

	while( *Current == -1 && Current < End )
	{
		Current = GetNext();
	}

	if( *Current < 0 )
		return true;
	else
		return false;
}


inline void Prefix::TrimPrefix( int * RecStart )
{
	SeqWrap * aSeqWrap = NULL;

	int * RecPtr = NULL;
	int * OldPrefixPtr = NULL;
	
	aSeqWrap = new SeqWrap( RecStart );

	RecPtr = (*aSeqWrap).GetFirst();
	Current = GetFirst();

	if( *RecPtr != *Current )
	{
		End = Start;
		delete aSeqWrap;
		return;
	}

	OldPrefixPtr = Current;

	while( *RecPtr != -2  && *RecPtr==*Current )
	{
		OldPrefixPtr = Current;

		RecPtr = (*aSeqWrap).GetNext();
		if( Current>=End )
			break;
		Current = GetNext();
	}
	//Current = OldPrefixPtr;

	//while( (*Current>=0) && Current>Start )
	//	Current--;

	while( (*OldPrefixPtr>=0) && OldPrefixPtr>Start )
		OldPrefixPtr--;

	End = OldPrefixPtr;
	if( Start < End )
	{
		int * LastNonEmpty = NULL;
		bool Tmp = false;

		Current = GetFirst();
		while( *Current >= 0 ) // Skip the first itemset.
			Current = GetNext();

		LastNonEmpty = Current;
		while( Current < OldPrefixPtr )
		{
			Tmp = CurItemSetIsEmpty;
			Current = GetNext();
			if( *Current == -1 && !Tmp )
			{
				LastNonEmpty = Current;
			}
		}

		End = LastNonEmpty;
	}
	delete aSeqWrap;
}

//************************
//************************

//************************
//************************

//************************
//************************

//************************
//************************


// This Proc assumes that all the records in pDB has a common Prefix PrefixSeq with lenght of PrefixLen.
// It creates and returns projected db for that common prefix.
struct PROJ_DB * MakeProjDB( const struct PROJ_DB* pDB, Prefix * aPrefix )
{
	int * PostStart = NULL;
	SeqWrap * aSeqWrap = NULL;
	//int * RecEnd;
	int * tmpPtr = NULL;
	int i = 0;
	int j = 0;
	int tmp = 0;
	int PatLen = 0;
	int Sup = 0;
	struct PROJ_DB * ProjDB = NULL;
	struct PROJ_SEQ * tmpSeq = NULL;

//	RTATest( pDB );
//	if( (*pDB).m_nPatLen>2 && (*pDB).m_pnPat[0] == 33 && (*pDB).m_pnPat[2] == 56  )
//		printf( " werwrwrwrewerwrwrwrwer\n" );

	//if( (*pDB).m_nPatLen>4 && (*pDB).m_pnPat[0] == 867 && (*pDB).m_pnPat[2] == 92 && (*pDB).m_pnPat[4] == 631 )
	//	printf( " werwrwrwrewerwrwrwrwer\n" );



	tmp = sizeof( struct PROJ_DB );
	n_total_mem += tmp;
  ProjDB = (struct PROJ_DB*) memalloc(tmp);

	PatLen = (*aPrefix).Size() + (*pDB).m_nPatLen;


	(*ProjDB).m_nPatLen = PatLen;

	// Copy the prefix of pDB to ProjDB's prefix.
	tmp = PatLen * sizeof(int);
	n_total_mem += tmp;
  (*ProjDB).m_pnPat = (int*) memalloc(tmp);
	if( (*pDB).m_nPatLen == 1 ) 
		(*ProjDB).m_pnPat[0] = (int) (*pDB).m_pnPat;
	else
		memcpy( (*ProjDB).m_pnPat, (*pDB).m_pnPat, (*pDB).m_nPatLen * sizeof(int) );

	// Add aPrefix to ProjDB's prefix.
	tmpPtr = (*aPrefix).GetFirst();

	for( i = (*pDB).m_nPatLen; i<PatLen; i++ )
	{
		(*ProjDB).m_pnPat[i] = (int) *tmpPtr;
		tmpPtr = (*aPrefix).GetNext();
	}

	(*ProjDB).m_nMaxSup = (*pDB).m_nSup;
	tmp = (*pDB).m_nSup * sizeof( struct PROJ_SEQ );
	n_total_mem += tmp;
	(*ProjDB).m_pProjSeq = (struct PROJ_SEQ*) memalloc( tmp );
	memset( (*ProjDB).m_pProjSeq, 0, tmp );
	(*ProjDB).m_nVer = -1;
	(*ProjDB).m_nSup = 0;


	int JMax = 1;
	int PrjCnt;
	bool Inter;
	bool IncSup;
	if( *(*aPrefix).GetFirst() == -1 )
		Inter = true;
	else
		Inter = false;

	#if defined( _USE_STRING_ELEMINATION )
	bool CheckStrings = false;
	if( (*MainSeqTree).IsContained( new Sequence(pDB) ) )
	{
		CheckStrings = true;
	}

	#endif // defined( _USE_STRING_ELEMINATION )

	for( i=0; i<(*pDB).m_nSup; i++ ) // For every seq in DB
	{

		 
		PrjCnt = 0;
		if( !Inter )
			JMax = (*pDB).m_pProjSeq[i].m_nProjCount;


		IncSup = true;

		#if defined( _USE_STRING_ELEMINATION )
		//if( (*MainSeqTree).IsContained( pDB, (*aSeqWrap).GetFirst() ) )
		if( CheckStrings )
		{
			if( (*MainSeqTree).IsContained( pDB, GetStartPtr( pDB, i) ) )
			{
				//Sequence * aS = new Sequence( pDB, GetStartPtr( pDB, i) );
				//(*aS).Print();
				//printf( " Sequence Eleminated.\n" );
				continue;
			}
		}
		#endif // defined( _USE_STRING_ELEMINATION )

		for( j=0; j < JMax; j++ ) 
		{
			aSeqWrap = new SeqWrap( GetStartPtr( pDB, i, j ) );

			if( !(*aSeqWrap).IsEmpty() )
			{


				if( IncSup )
				{
					Sup++;
					IncSup = false;
				}

				PostStart = (*aSeqWrap).GetItemSet( (*aPrefix).NumOfItemSets );
/*
				Sequence * aSeq = NULL;
				aSeq = new Sequence( GetStartPtr( pDB, i ), false );
				printf( "Rec:\n  " );
				(*aSeq).Print();
				delete aSeq;
				aSeq = new Sequence( PostStart, false );
				printf( "Post:\n  " );
				(*aSeq).Print();
				delete aSeq;
*/

				if( !SeqIsEmpty( PostStart ) )
				{
					buf_idx[ PrjCnt++ ] = int( PostStart );
				}
			
			}
			delete aSeqWrap;
		}
		if( PrjCnt > 0 )
		{
			tmpSeq = (struct PROJ_SEQ *) (*ProjDB).m_pProjSeq + (*ProjDB).m_nSup;
			(*ProjDB).m_nSup++;
			(*tmpSeq).m_nProjCount = PrjCnt;
			if( PrjCnt == 1 )
			{
				(*tmpSeq).m_ppSeq = (int**) buf_idx[0];
			} else {
				tmp = PrjCnt * sizeof(int);
				n_total_mem += tmp;
				(*tmpSeq).m_ppSeq = (int**) memalloc( tmp );
				memcpy( (*tmpSeq).m_ppSeq, buf_idx, tmp );
			}
		}
	}

#if defined( _USE_OUTPUT_BUFFER )
	(*ProjDB).OutputBuf = OutputPattern( (*ProjDB).m_pnPat, PatLen, Sup );
	(*(*ProjDB).OutputBuf).Parent = (*pDB).OutputBuf;
	(*(*pDB).OutputBuf).ReferenceCount++;
#else
	OutputPattern( (*ProjDB).m_pnPat, PatLen, Sup );
#endif


	if( (*ProjDB).m_nSup < gSUP  )
	{

		//Added by Ramin
#if defined( _USE_OUTPUT_BUFFER )
				//fprintf( gpFreqFile, "+++===>>>>>>  Right Pattern  " );
				//(*(*ProjDB).OutputBuf).Print( gpFreqFile );
				EmptyBuffer( aSeqList, (*ProjDB).OutputBuf );
#else
				if( n_max_mem < n_total_mem )
					n_max_mem = n_total_mem;
#endif
#if defined( _FIND_MAX_SEQS ) && !defined( _DO_NAIVE_APPROACH )
				//Sequence * aSeq = new Sequence( (*ProjDB).m_pnPat, (*ProjDB).m_nPatLen, (*ProjDB).m_nSup );
				Sequence * aSeq = new Sequence( ProjDB, Sup );
				(*MainSeqTree).AddSeq( aSeq );
#endif


		tmp = (*ProjDB).m_nMaxSup * sizeof( struct PROJ_SEQ );
		n_total_mem -= tmp;
		freemem( (void**) &(*ProjDB).m_pProjSeq );

		tmp = PatLen * sizeof(int);
		n_total_mem -= tmp;
		freemem( (void**)&((*ProjDB).m_pnPat) );

		n_total_mem -= sizeof( struct PROJ_DB );
	  freemem( (void**)&(ProjDB) );
		return NULL;
	} else
		return ProjDB;
}


////////////////////////////////////////
////////////////////////////////////////
// This Proc assumes that all the records in pDB has a common Prefix PrefixSeq with lenght of PrefixLen.
// It creates and returns projected db for that common prefix.
//struct PROJ_DB * MakeProjDB123( const struct PROJ_DB* pDB, Prefix * aPrefix )
struct PROJ_DB * MakeProjDBFromOrg(  const struct mem_map * pDB, Prefix * aPrefix, long nCount )
{
	int * PostStart = NULL;
	SeqWrap * aSeqWrap = NULL;
	//int * RecEnd;
	int * tmpPtr = NULL;
	int i = 0;
	int j = 0;
	int tmp = 0;
	int PatLen = 0;
	int Sup = 0;
	struct PROJ_DB * ProjDB = NULL;
	struct PROJ_SEQ * tmpSeq = NULL;


	tmp = sizeof( struct PROJ_DB );
	n_total_mem += tmp;
  ProjDB = (struct PROJ_DB*) memalloc(tmp);

	PatLen = (*aPrefix).Size();


	(*ProjDB).m_nPatLen = PatLen;

	// Copy the prefix of pDB to ProjDB's prefix.
	tmp = PatLen * sizeof(int);
	n_total_mem += tmp;
  (*ProjDB).m_pnPat = (int*) memalloc(tmp);

	// Add aPrefix to ProjDB's prefix.
	tmpPtr = (*aPrefix).GetFirst();
	while( *tmpPtr < 0 && tmpPtr < (*aPrefix).End )
		tmpPtr = GetNextItem( tmpPtr+1 ); // Get first frequent item.

	for( i = 0; i<PatLen; i++ )
	{
		(*ProjDB).m_pnPat[i] = (int) *tmpPtr;
		tmpPtr = (*aPrefix).GetNext();
	}

	(*ProjDB).m_nMaxSup = nCount;
	tmp = nCount * sizeof( struct PROJ_SEQ );
	n_total_mem += tmp;
	(*ProjDB).m_pProjSeq = (struct PROJ_SEQ*) memalloc( tmp );
	memset( (*ProjDB).m_pProjSeq, 0, tmp );
	(*ProjDB).m_nVer = -1;
	(*ProjDB).m_nSup = 0;

//'''''''''''''''''''''''''''''''''''''''

  int * RecStart = (int*) GetStartOfMap( pDB );
  int *LastAddr = (int*) GetLastAddrOfMap( pDB );

	while( RecStart < LastAddr )
	{
		aSeqWrap = new SeqWrap( RecStart );

		if( !(*aSeqWrap).IsEmpty() )
		{
			Sup++;

			PostStart = (*aSeqWrap).GetItemSet( (*aPrefix).NumOfItemSets );

			if( !SeqIsEmpty( PostStart ) )
			{
				tmpSeq = (struct PROJ_SEQ *) (*ProjDB).m_pProjSeq + (*ProjDB).m_nSup;
				(*ProjDB).m_nSup++;

				(*tmpSeq).m_nProjCount = 1;

				(*tmpSeq).m_ppSeq = (int**) PostStart;
			}
		
		}
		delete aSeqWrap;

		RecStart = GetEndPtr(RecStart) + 1;
	}


#if defined( _USE_OUTPUT_BUFFER )
	(*ProjDB).OutputBuf = OutputPattern( (*ProjDB).m_pnPat, PatLen, Sup );
#else
	OutputPattern( (*ProjDB).m_pnPat, PatLen, Sup );
#endif


	if( (*ProjDB).m_nSup < gSUP  )
	{

		//Added by Ramin
#if defined( _USE_OUTPUT_BUFFER )
				//fprintf( gpFreqFile, "+++===>>>>>>  Right Pattern  " );
				//(*(*ProjDB).OutputBuf).Print( gpFreqFile );
				EmptyBuffer( aSeqList, (*ProjDB).OutputBuf );
#else
				if( n_max_mem < n_total_mem )
					n_max_mem = n_total_mem;
#endif
#if defined( _FIND_MAX_SEQS ) && !defined( _DO_NAIVE_APPROACH )
				//Sequence * aSeq = new Sequence( (*ProjDB).m_pnPat, (*ProjDB).m_nPatLen, (*ProjDB).m_nSup );
				Sequence * aSeq = new Sequence( ProjDB, Sup );
				(*MainSeqTree).AddSeq( aSeq );
#endif


		tmp = (*ProjDB).m_nMaxSup * sizeof( struct PROJ_SEQ );
		n_total_mem -= tmp;
		freemem( (void**) &(*ProjDB).m_pProjSeq );

		tmp = PatLen * sizeof(int);
		n_total_mem -= tmp;
		freemem( (void**)&((*ProjDB).m_pnPat) );

		n_total_mem -= sizeof( struct PROJ_DB );
	  freemem( (void**)&(ProjDB) );
		return NULL;
	} else
		return ProjDB;
}

#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )
