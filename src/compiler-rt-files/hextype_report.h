#include "hextype_rbtree.h"

#ifdef HEX_LOG
#include <atomic>

#define MAXINDEX 1000
#define MAXLEN 1000

#define numUpdate 1
#define numUpdateHit 2
#define numUpdateMiss 3

#define numGloUp 4
#define numStackUp 5
#define numStackRm 6
#define numHeapUp 7
#define numHeapRm 8

#define numRemoveHit 9
#define numRemoveMiss 10

#define numCasting 11
#define numVerifiedCasting 12
#define numCastingNull 13
#define numHashMapNull 14

#define numLookHit 15
#define numLookMiss 16
#define numLookFail 17

#define numCastSame 18
#define numCastHit 19
#define numCastMiss 20
#define numCastNoCacheUse 21

#define numCastBadCast 22
#define numCastNonBadCast 23
#define numMissFindObj 24
#define numCastBadCastMinus 25

#define numrealloc 26

#define numdynamicCast 27
#define numstaticCast 28
#define numreinterpretCast 29
#define numplacementNew 30

#define numRulesNull 31

#define numBadCastType1 32
#define numBadCastType2 33
#define numBadCastType3 34
#define numBadCastType4 35

void IncVal(int index, int count);
unsigned long getVal(int index);
void printTypeConfusion(int, uint64_t, uint64_t);
void InstallAtExitHandler();
#endif
