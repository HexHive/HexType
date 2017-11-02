#include "hextype_report.h"
#include <unordered_map>

#define NUMMAP 268435460
#define NUMCACHE 16777220

#define BADCAST 0
#define FAILINFO 1
#define SAFECASTSAME 2
#define SAFECASTUPCAST 3
#define SAFECAST 4

#define STACKALLOC 1
#define HEAPALLOC 2
#define GLOBALALLOC 3
#define REALLOC 4
#define PLACEMENTNEW 5
#define REINTERPRET 6

inline uint32_t getHash(uptr a) {
  return (((a >> 3) & 0xfffffff));
}

// Stoarge for phantom class information
typedef std::set<uint64_t> PhantomHashSet;
static std::unordered_map<uint64_t, PhantomHashSet*> *ObjPhantomInfo;

__attribute__ ((visibility ("default"))) ObjTypeMapEntry *ObjTypeMap;
__attribute__ ((visibility ("default"))) VerifyResultEntry *VerifyResultCache;
