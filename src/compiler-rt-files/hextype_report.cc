#include "hextype_report.h"
#include <inttypes.h>
#include <execinfo.h>
#include <unistd.h>

#ifdef HEX_LOG
#define BT_BUF_SIZE 100

std::atomic<unsigned long> count_index[MAXINDEX];

void IncVal(int index, int count) {
  count_index[index].fetch_add(count);
}

unsigned long getVal(int index) {
  return count_index[index].load();
}

void printInfotoFile(char *PrintStr, char *FileName) {
  write_log(PrintStr, FileName);
}

void printTypeConfusion(int ErrorType, uint64_t SrcHash, uint64_t DstHash) {
  int j, nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;

#ifdef PRINT_BAD_CASTING
  printf("== HexType Type Confusion Report ==\n");
  printf("%d %" PRIu64 " %" PRIu64 "\n", ErrorType, SrcHash, DstHash);

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  strings = backtrace_symbols(buffer, nptrs);
  if (strings != NULL)
    for (j = 0; j < nptrs; j++)
      printf("%s\n", strings[j]);
  free(strings);
#endif
#ifdef PRINT_BAD_CASTING_FILE
  char tmp[MAXLEN];
  char fileName[MAXLEN] = "/typeconfusion.txt";

  snprintf(tmp, sizeof(tmp), "\n== HexType Type Confusion Report ==");
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%d %" PRIu64 " %" PRIu64 "",
           ErrorType, SrcHash, DstHash);
  printInfotoFile(tmp, fileName);

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  strings = backtrace_symbols(buffer, nptrs);
  if (strings != NULL)
    for (j = 0; j < nptrs; j++) {
      snprintf(tmp, sizeof(tmp), "%s", strings[j]);
      printInfotoFile(tmp, fileName);
    }
  free(strings);
#endif

#ifdef PRINT_BAD_CASTING_FATAL
  TERMINATE
#endif
}

static void PrintStatResult(void) {
  char tmp[MAXLEN];
  char fileName[MAXLEN] = "/total_result.txt";

  snprintf(tmp, sizeof(tmp), "== Object Update status ==\n");
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: Object update\n", getVal(numGloUp) +
          getVal(numHeapUp) + getVal(numStackUp));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Object update hit\n", getVal(numGloUp) +
          getVal(numHeapUp) + getVal(numStackUp) - getVal(numUpdateMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Object update miss\n",
           getVal(numUpdateMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Global Object Update\n",
          getVal(numGloUp));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Heap Object Update\n", getVal(numHeapUp));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Stack Object Update\n",
          getVal(numStackUp));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: Object remove\n", getVal(numHeapRm) +
          getVal(numStackRm));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Object remove hit\n", getVal(numHeapRm) +
          getVal(numStackRm) - getVal(numRemoveMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Object remove miss\n",
           getVal(numRemoveMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Heap object Remove\n", getVal(numHeapRm));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Stack object Remove\n",getVal(numStackRm));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "== Casting verification status ==\n");
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu (verified %lu): Casting operation\n",
           getVal(numCasting), getVal(numVerifiedCasting));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu: Object lookup success\n",
           getVal(numLookHit));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t%lu: Object lookup success (find in the RB-tree)\n",
          getVal(numLookMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t%lu: Object lookup fail (fail to find object)\n",
           getVal(numLookFail));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu %lu: Verified type casting\n",
          getVal(numVerifiedCasting),
          getVal(numCastNonBadCast) +
          getVal(numCastSame) +
          getVal(numCastBadCast)+getVal(numMissFindObj));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t\t%lu: Safe casting (src and dst are same type)\n",
           getVal(numCastSame));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t\t%lu: Safe casting (up casting)\n",
          getVal(numCastNonBadCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t\t%lu: Type confusion cases\n",getVal(numCastBadCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t\t%lu: Type confusion type %lu %lu %lu %lu\n",
           getVal(numBadCastType1),
           getVal(numBadCastType2),
           getVal(numBadCastType3),
           getVal(numBadCastType4));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t\t\t%lu: Type confusion cases(minus offset)\n",
          getVal(numCastBadCastMinus));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp),
           "\t\t%lu: No type relation info\n",getVal(numMissFindObj));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t%lu :Casting cache use status\n",
           getVal(numCastHit) + getVal(numCastMiss) +
           getVal(numCastNoCacheUse));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t\t%lu: Casting operation cache hit\n",
          getVal(numCastHit));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t\t%lu: Casting operation cache Miss\n",
          getVal(numCastMiss));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "\t\t%lu: Casting operation no cache use\n",
          getVal(numCastNoCacheUse));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "== Count dynamic and static cast number ==\n");
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: Check dynamic_cast number\n",
          getVal(numdynamicCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: Check static_cast number\n",
           getVal(numstaticCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: Check reinterpret_cast number\n",
          getVal(numreinterpretCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: reinterpret_cast objects\n",
          getVal(numreinterpretCast));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "%lu: placement new objects\n",
          getVal(numplacementNew));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "== Summary == \n");
  snprintf(tmp, sizeof(tmp), "Objects: %lu %lu %lu\n",
           getVal(numGloUp) + getVal(numHeapUp) + getVal(numStackUp),
           getVal(numHeapRm) + getVal(numStackRm),
          getVal(numGloUp) + getVal(numHeapUp) + getVal(numStackUp) +
          getVal(numreinterpretCast) + getVal(numplacementNew));
  printInfotoFile(tmp, fileName);

  snprintf(tmp, sizeof(tmp), "Casting: %lu %lu %lu\n",
           getVal(numCasting), getVal(numVerifiedCasting), getVal(numCastBadCast));
  printInfotoFile(tmp, fileName);
}

static void HexTypeAtExit(void) {
  PrintStatResult();
}

void InstallAtExitHandler() {
  atexit(HexTypeAtExit);
}
#endif
