//===-- hextype.cc -- runtime support for HexType  ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===-------------------------------------------------------------------===//

#include "hextype.h"
#include <string.h>

__attribute__((always_inline))
  inline ObjTypeMapEntry *findObjInfo(uptr* SrcAddr) {
    uint32_t MapIndex = getHash((uptr)SrcAddr);
    if (ObjTypeMap[MapIndex].ObjAddr == SrcAddr) {
#ifdef HEX_LOG
      IncVal(numLookHit, 1);
#endif
      return &ObjTypeMap[MapIndex];
    }

    if (ObjTypeMap[MapIndex].HexTree != nullptr &&
        ObjTypeMap[MapIndex].HexTree->root != nullptr) {
      ObjTypeMapEntry *FindValue =
        (ObjTypeMapEntry *)rbtree_lookup(ObjTypeMap[MapIndex].HexTree, SrcAddr);
#ifdef HEX_LOG
      if (FindValue != nullptr)
        IncVal(numLookMiss, 1);
      else
        IncVal(numLookFail, 1);
#endif
      return FindValue;
    }
#ifdef HEX_LOG
    IncVal(numLookFail, 1);
#endif
    return nullptr;
  }

__attribute__((always_inline))
  inline static void* verifyTypeCasting(uptr* const SrcAddr,
                                 uptr* const DstAddr,
                                 const uint64_t DstTypeHashValue) {
    if(SrcAddr == NULL) return nullptr;
#ifdef HEX_LOG
    IncVal(numCasting, 1);
#endif
    ObjTypeMapEntry *FindValue = findObjInfo(SrcAddr);
    if (!FindValue)
      return DstAddr;
#ifdef HEX_LOG
    IncVal(numVerifiedCasting, 1);
#endif
    if (DstAddr != SrcAddr) {
      int OffsetTmp = FindValue->Offset;
      if (OffsetTmp == -1)
        OffsetTmp = 0;
      long offset = ((char *)DstAddr - ((char *)SrcAddr - OffsetTmp));

      FindValue = findObjInfo(DstAddr);
      if (offset < 0) {
        if (FindValue) {
          uint64_t SrcTypeHashValue = FindValue->TypeHashValue;
          if (SrcTypeHashValue == DstTypeHashValue) {
#ifdef HEX_LOG
            IncVal(numCastSame, 1);
#endif
            return DstAddr;
          }
        }
#ifdef HEX_LOG
        IncVal(numCastBadCast, 1);
        IncVal(numCastBadCastMinus, 1);
#endif
#if defined(PRINT_BAD_CASTING) || defined(PRINT_BAD_CASTING_FILE)
        IncVal(numBadCastType1, 1);
        printTypeConfusion(1, 0, DstTypeHashValue);
#endif
        return nullptr;
      }
      if (!FindValue) {
#ifdef HEX_LOG
        IncVal(numCastBadCast, 1);
        IncVal(numCastBadCastMinus, 1);
#endif
#if defined(PRINT_BAD_CASTING) || defined(PRINT_BAD_CASTING_FILE)
        IncVal(numBadCastType2, 1);
        printTypeConfusion(2, 0, DstTypeHashValue);
#endif
        return nullptr;
      }
    }

    uint64_t SrcTypeHashValue = FindValue->TypeHashValue;
    uint64_t CacheIndex;
    CacheIndex = (SrcTypeHashValue & 0xfff);
    CacheIndex <<= 12;
    CacheIndex |= (DstTypeHashValue & 0xfff);

    if (SrcTypeHashValue == DstTypeHashValue) {
#ifdef HEX_LOG
      IncVal(numCastNoCacheUse, 1);
      IncVal(numCastSame, 1);
#endif
      VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
      VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
      VerifyResultCache[CacheIndex].VerifyResult = SAFECASTSAME;
      return DstAddr;
    }

    if (VerifyResultCache[CacheIndex].SrcHValue == SrcTypeHashValue &&
        VerifyResultCache[CacheIndex].DstHValue == DstTypeHashValue) {
#ifdef HEX_LOG
      IncVal(numCastHit, 1);
#endif
      if (VerifyResultCache[CacheIndex].VerifyResult != BADCAST) {
#ifdef HEX_LOG
        char VerifyResult = VerifyResultCache[CacheIndex].VerifyResult;
        switch (VerifyResult) {
        case SAFECASTUPCAST:
          IncVal(numCastNonBadCast, 1);
          break;
        case SAFECASTSAME:
          IncVal(numCastSame, 1);
          break;
        case FAILINFO:
          IncVal(numMissFindObj, 1);
          break;
        }
#endif
        return DstAddr;
      }
    }

    else {
#ifdef HEX_LOG
      IncVal(numCastMiss, 1);
#endif
      uptr* RuleAddr = FindValue->RuleAddr;
      if (RuleAddr) {
        uint64_t RuleHash;
        char *BaseAddr = (char *)RuleAddr;
        uint64_t RuleSize = *(FindValue->RuleAddr);
        uint64_t start = 1, end = RuleSize, middle;
        middle = (start + end) / 2;

        while (start <= end) {
          RuleHash = *((uint64_t *)(BaseAddr + (sizeof(uint64_t) * middle)));

          if (RuleHash < DstTypeHashValue)
            start = middle + 1;
          else if(RuleHash == DstTypeHashValue) {
            VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
            VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
            VerifyResultCache[CacheIndex].VerifyResult = SAFECASTUPCAST;
#ifdef HEX_LOG
            IncVal(numCastNonBadCast, 1);
#endif
            return DstAddr;
          }
          else
            end = middle - 1;
          middle = (start + end) /2;
        }
      } else {
        VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
        VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
        VerifyResultCache[CacheIndex].VerifyResult = FAILINFO;
#ifdef HEX_LOG
        IncVal(numMissFindObj, 1);
#endif
        return nullptr;
      }

      std::unordered_map<uint64_t, PhantomHashSet*>::iterator it;
      it = ObjPhantomInfo->find(DstTypeHashValue);
      if (it != ObjPhantomInfo->end()) {
        PhantomHashSet *TargetPhantomHashSet = it->second;
        char *BaseAddr = (char *)RuleAddr;
        uint64_t RuleSize = *(FindValue->RuleAddr);
        uint64_t RuleHash;
        for (uint64_t PhantomHash : *TargetPhantomHashSet) {
          uint64_t start = 1, end = RuleSize, middle;
          middle = (start + end) / 2;
          while (start <= end) {
            RuleHash = *((uint64_t *)(BaseAddr + (sizeof(uint64_t) * middle)));
            if (RuleHash < PhantomHash)
              start = middle + 1;
            else if(RuleHash == PhantomHash) {
              VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
              VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
              VerifyResultCache[CacheIndex].VerifyResult = SAFECASTUPCAST;
#ifdef HEX_LOG
              IncVal(numCastNonBadCast, 1);
#endif
              return DstAddr;
            }
            else
              end = middle - 1;
            middle = (start + end) /2;
          }
        }
      }

      VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
      VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
      VerifyResultCache[CacheIndex].VerifyResult = BADCAST;
    }

#if defined(PRINT_BAD_CASTING) || defined(PRINT_BAD_CASTING_FILE)
      IncVal(numBadCastType3, 1);
      printTypeConfusion(3, SrcTypeHashValue, DstTypeHashValue);
#endif

#ifdef HEX_LOG
    IncVal(numCastBadCast, 1);
#endif
    return nullptr;
  }

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __type_casting_verification_inline(const uint64_t SrcTypeHashValue,
                                         const uint64_t DstTypeHashValue,
                                         const uint64_t ObjMapIndex,
                                         const uint64_t CacheIndex) {
#ifdef HEX_LOG
  IncVal(numCasting, 1);
  IncVal(numVerifiedCasting, 1);
  IncVal(numLookHit, 1);
  IncVal(numCastMiss, 1);
#endif

  if (SrcTypeHashValue == DstTypeHashValue) {
#ifdef HEX_LOG
    IncVal(numCastSame, 1);
#endif
    VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
    VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
    VerifyResultCache[CacheIndex].VerifyResult = SAFECASTSAME;
    return;
  }

  ObjTypeMapEntry *FindValue = &ObjTypeMap[ObjMapIndex];
  uptr* RuleAddr = FindValue->RuleAddr;
  if (RuleAddr) {
    uint64_t RuleHash;
    char *BaseAddr = (char *)RuleAddr;
    uint64_t RuleSize = *(FindValue->RuleAddr);
    uint64_t start = 1, end = RuleSize, middle;
    middle = (start + end) / 2;

    while (start <= end) {
      RuleHash = *((uint64_t *)(BaseAddr + (sizeof(uint64_t) * middle)));

      if (RuleHash < DstTypeHashValue)
        start = middle + 1;
      else if(RuleHash == DstTypeHashValue) {
        VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
        VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
        VerifyResultCache[CacheIndex].VerifyResult = SAFECASTUPCAST;
#ifdef HEX_LOG
        IncVal(numCastNonBadCast, 1);
#endif
        return;
      }
      else
        end = middle - 1;
      middle = (start + end) /2;
    }
  } else {
    VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
    VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
    VerifyResultCache[CacheIndex].VerifyResult = FAILINFO;
#ifdef HEX_LOG
    IncVal(numMissFindObj, 1);
#endif
    return;
  }

  std::unordered_map<uint64_t, PhantomHashSet*>::iterator it;
  it = ObjPhantomInfo->find(DstTypeHashValue);
  if (it != ObjPhantomInfo->end()) {
    PhantomHashSet *TargetPhantomHashSet = it->second;
    char *BaseAddr = (char *)RuleAddr;
    uint64_t RuleSize = *(FindValue->RuleAddr);
    uint64_t RuleHash;
    for (uint64_t PhantomHash : *TargetPhantomHashSet) {
      uint64_t start = 1, end = RuleSize, middle;
      middle = (start + end) / 2;
      while (start <= end) {
        RuleHash = *((uint64_t *)(BaseAddr + (sizeof(uint64_t) * middle)));
        if (RuleHash < PhantomHash)
          start = middle + 1;
        else if(RuleHash == PhantomHash) {
          VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
          VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
          VerifyResultCache[CacheIndex].VerifyResult = SAFECASTUPCAST;
#ifdef HEX_LOG
          IncVal(numCastNonBadCast, 1);
#endif
          return;
        }
        else
          end = middle - 1;
        middle = (start + end) /2;
      }
    }
  }

  VerifyResultCache[CacheIndex].SrcHValue = SrcTypeHashValue;
  VerifyResultCache[CacheIndex].DstHValue = DstTypeHashValue;
  VerifyResultCache[CacheIndex].VerifyResult = BADCAST;
#if defined(PRINT_BAD_CASTING) || defined(PRINT_BAD_CASTING_FILE)
  IncVal(numBadCastType4, 1);
  printTypeConfusion(4, SrcTypeHashValue, DstTypeHashValue);
#endif
#ifdef HEX_LOG
  IncVal(numCastBadCast, 1);
#endif
  return;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __type_casting_verification_print_cache_result(const uint64_t index) {
#ifdef PRINT_BAD_CASTING
  printf("== HexType Type confusion Report\n");
#endif
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __type_casting_verification_inline_normal(uptr* const SrcAddr,
                                               const uint64_t DstTypeHashValue) {
  verifyTypeCasting(SrcAddr, SrcAddr, DstTypeHashValue);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __type_casting_verification(uptr* const SrcAddr,
                                       const uint64_t DstTypeHashValue) {
  verifyTypeCasting(SrcAddr, SrcAddr, DstTypeHashValue);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __type_casting_verification_changing(uptr* const SrcAddr,
                                          uptr* const DstAddr,
                                          const uint64_t DstTypeHashValue) {
  verifyTypeCasting(SrcAddr, DstAddr, DstTypeHashValue);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void* __dynamic_casting_verification(uptr* const SrcAddr,
                                     const uint64_t DstTypeHashValue,
                                     std::ptrdiff_t Src2dst_offset) {
  uptr* TmpAddr = (uptr *)((char *)SrcAddr - Src2dst_offset);
  return verifyTypeCasting(SrcAddr, TmpAddr, DstTypeHashValue);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __update_direct_oinfo(uptr* const AllocAddr, const uint64_t TypeHashValue,
                           const int Offset,
                           uptr* const RuleAddr) {
  uptr MapIndex = getHash((uptr)AllocAddr);
  if (ObjTypeMap[MapIndex].ObjAddr == nullptr ||
      ObjTypeMap[MapIndex].ObjAddr == AllocAddr) {
    ObjTypeMap[MapIndex].ObjAddr = AllocAddr;
    ObjTypeMap[MapIndex].TypeHashValue = TypeHashValue;
    ObjTypeMap[MapIndex].Offset = Offset;
    ObjTypeMap[MapIndex].HeapArraySize = 1;
    ObjTypeMap[MapIndex].RuleAddr = RuleAddr;
    return;
  }
#ifdef HEX_LOG
  IncVal(numUpdateMiss, 1);
#endif
  if (ObjTypeMap[MapIndex].HexTree == nullptr)
    ObjTypeMap[MapIndex].HexTree = rbtree_create();

  ObjTypeMapEntry *ObjValue =
    (ObjTypeMapEntry*)malloc(sizeof(ObjTypeMapEntry));
  memcpy(ObjValue, &ObjTypeMap[MapIndex], sizeof(ObjTypeMapEntry));

  rbtree_insert(ObjTypeMap[MapIndex].HexTree,
                ObjTypeMap[MapIndex].ObjAddr, ObjValue);
  ObjTypeMap[MapIndex].ObjAddr = AllocAddr;
  ObjTypeMap[MapIndex].TypeHashValue = TypeHashValue;
  ObjTypeMap[MapIndex].Offset = Offset;
  ObjTypeMap[MapIndex].HeapArraySize = 1;
  ObjTypeMap[MapIndex].RuleAddr = RuleAddr;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __update_direct_oinfo_inline(uptr* const AllocAddr,
                                  const uint64_t TypeHashValue,
                                  const int Offset,
                                  uptr* RuleAddr,
                                  const uint64_t MapIndex) {
#ifdef HEX_LOG
  IncVal(numUpdateMiss, 1);
#endif
  if (ObjTypeMap[MapIndex].HexTree == nullptr)
    ObjTypeMap[MapIndex].HexTree = rbtree_create();

  ObjTypeMapEntry *ObjValue =
    (ObjTypeMapEntry*)malloc(sizeof(ObjTypeMapEntry));
  memcpy(ObjValue, &ObjTypeMap[MapIndex], sizeof(ObjTypeMapEntry));

  rbtree_insert(ObjTypeMap[MapIndex].HexTree,
                ObjTypeMap[MapIndex].ObjAddr, ObjValue);
  ObjTypeMap[MapIndex].ObjAddr = AllocAddr;
  ObjTypeMap[MapIndex].TypeHashValue = TypeHashValue;
  ObjTypeMap[MapIndex].Offset = Offset;
  ObjTypeMap[MapIndex].HeapArraySize = 1;
  ObjTypeMap[MapIndex].RuleAddr = RuleAddr;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __handle_reinterpret_cast(uptr* const AllocAddr,
                               const uint64_t TypeHashValue,
                               const int Offset,
                               uptr* const RuleAddr) {
  ObjTypeMapEntry *FindValue = findObjInfo(AllocAddr);
  if (FindValue) {
    if (FindValue->Offset != -1)
      return;
    //  verifyTypeCasting(AllocAddr, AllocAddr, TypeHashValue);
  }
  __update_direct_oinfo(AllocAddr, TypeHashValue, -1, RuleAddr);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __update_oinfo(uptr* const AllocAddr, const uint64_t TypeHashValue,
                    const int Offset,
                    const uint32_t TypeSize, const unsigned long ArraySize,
                    uptr* const RuleAddr) {
  for (uint32_t i=0;i<ArraySize;i++) {
    uptr *addr = (uptr *)((char *)AllocAddr + (TypeSize*i));
    uptr MapIndex = getHash((uptr)addr);

    if (ObjTypeMap[MapIndex].ObjAddr == nullptr ||
        ObjTypeMap[MapIndex].ObjAddr == addr) {
      ObjTypeMap[MapIndex].ObjAddr = addr;
      ObjTypeMap[MapIndex].TypeHashValue = TypeHashValue;
      ObjTypeMap[MapIndex].Offset = Offset;
      ObjTypeMap[MapIndex].HeapArraySize  = ArraySize;
      ObjTypeMap[MapIndex].RuleAddr = RuleAddr;
      continue;
    }

    else {
#ifdef HEX_LOG
      IncVal(numUpdateMiss, 1);
#endif
      if (ObjTypeMap[MapIndex].HexTree == nullptr)
        ObjTypeMap[MapIndex].HexTree = rbtree_create();

      ObjTypeMapEntry *ObjValue =
        (ObjTypeMapEntry*)malloc(sizeof(ObjTypeMapEntry));
      memcpy(ObjValue, &ObjTypeMap[MapIndex], sizeof(ObjTypeMapEntry));

      rbtree_insert(ObjTypeMap[MapIndex].HexTree,
                    ObjTypeMap[MapIndex].ObjAddr, ObjValue);
      ObjTypeMap[MapIndex].ObjAddr = addr;
      ObjTypeMap[MapIndex].TypeHashValue = TypeHashValue;
      ObjTypeMap[MapIndex].Offset = Offset;
      ObjTypeMap[MapIndex].HeapArraySize = ArraySize;
      ObjTypeMap[MapIndex].RuleAddr = RuleAddr;
    }
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __remove_direct_oinfo(uptr* const TargetAddr) {
  uptr MapIndex = getHash((uptr)TargetAddr);

  if (ObjTypeMap[MapIndex].ObjAddr == TargetAddr) {
    ObjTypeMap[MapIndex].ObjAddr = nullptr;
    return;
  }
#ifdef HEX_LOG
  IncVal(numRemoveMiss, 1);
#endif
  if (ObjTypeMap[MapIndex].HexTree != nullptr &&
      ObjTypeMap[MapIndex].HexTree->root != nullptr) {
    ObjTypeMapEntry* FindValue =
      (ObjTypeMapEntry *)rbtree_lookup(ObjTypeMap[MapIndex].HexTree, TargetAddr);
    if (FindValue != nullptr) {
      free(FindValue);
      rbtree_delete(ObjTypeMap[MapIndex].HexTree, TargetAddr);
    }
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __remove_direct_oinfo_inline(uptr* const TargetAddr,
                                  const uint64_t MapIndex) {
  if (ObjTypeMap[MapIndex].HexTree != nullptr &&
      ObjTypeMap[MapIndex].HexTree->root != nullptr) {
    ObjTypeMapEntry* FindValue =
      (ObjTypeMapEntry *)rbtree_lookup(ObjTypeMap[MapIndex].HexTree, TargetAddr);
    if (FindValue != nullptr) {
      free(FindValue);
      rbtree_delete(ObjTypeMap[MapIndex].HexTree, TargetAddr);
    }
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __remove_oinfo(uptr* const ObjectAddr, const uint32_t TypeSize,
                    unsigned long ArraySize, const uint32_t AllocType) {
  if (AllocType == HEAPALLOC || AllocType == REALLOC) {
    uptr MapIndex = getHash((uptr)ObjectAddr);
    if (ObjTypeMap[MapIndex].ObjAddr == ObjectAddr)
      ArraySize = ObjTypeMap[MapIndex].HeapArraySize;
    else {
      if (ObjTypeMap[MapIndex].HexTree != nullptr &&
          ObjTypeMap[MapIndex].HexTree->root != nullptr) {
        ObjTypeMapEntry* FindValue =
          (ObjTypeMapEntry *)rbtree_lookup(ObjTypeMap[MapIndex].HexTree, ObjectAddr);
        if (FindValue != nullptr)
          ArraySize = FindValue->HeapArraySize;
      }
      else
        ArraySize = 1;
    }
  }

  for (uint32_t i=0;i<ArraySize;i++) {
    uptr *addr = (uptr *)((char *)ObjectAddr + (TypeSize*i));
    uptr MapIndex = getHash((uptr)addr);
#ifdef HEX_LOG
    switch (AllocType) {
    case HEAPALLOC:
    case REALLOC:
      IncVal(numHeapRm, 1);
      break;
    }
#endif
    if (ObjTypeMap[MapIndex].ObjAddr == addr)
      ObjTypeMap[MapIndex].ObjAddr = nullptr;
    else {
#ifdef HEX_LOG
      IncVal(numRemoveMiss, 1);
#endif
      if (ObjTypeMap[MapIndex].HexTree != nullptr &&
          ObjTypeMap[MapIndex].HexTree->root != nullptr) {
        ObjTypeMapEntry* FindValue =
          (ObjTypeMapEntry *)rbtree_lookup(ObjTypeMap[MapIndex].HexTree, addr);
        if (FindValue != nullptr) {
          free(FindValue);
          rbtree_delete(ObjTypeMap[MapIndex].HexTree, addr);
        }
      }
    }
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __update_phantom_info(uint64_t *const PhantomInfo) {
  if (ObjTypeMap == nullptr) {
#ifdef HEX_LOG
    InstallAtExitHandler();
#endif
    ObjTypeMap = new ObjTypeMapEntry[NUMMAP];
  }

  if (VerifyResultCache == nullptr)
    VerifyResultCache = new VerifyResultEntry[NUMCACHE];

  if (ObjPhantomInfo == nullptr)
    ObjPhantomInfo = new std::unordered_map<uint64_t, PhantomHashSet*>;

  uint64_t pos = 0;
  uint64_t TotalNum = PhantomInfo[pos++];
  for (uint64_t i=0;i<TotalNum;i++) {
    uint64_t TypeHash = PhantomInfo[pos++];
    uint64_t PhantomNum = PhantomInfo[pos++];
    PhantomHashSet* PhantomSet;
    auto it = ObjPhantomInfo->find(TypeHash);
    bool isExist = false;
    if (it == ObjPhantomInfo->end()) {
      PhantomSet = new PhantomHashSet();
    } else {
      isExist = true;
      PhantomSet = it->second;
    }
    for (uint64_t j=0;j<PhantomNum;j++) {
      uint64_t innerHash = PhantomInfo[pos++];
      auto it2 = ObjPhantomInfo->find(innerHash);
      if (it2 != ObjPhantomInfo->end()) {
        if (it2->second != NULL && it2->second != PhantomSet)
          PhantomSet->insert(it2->second->begin(), it2->second->end());
        ObjPhantomInfo->erase(it2);
        ObjPhantomInfo->insert(make_pair(innerHash, PhantomSet));
      }
      PhantomSet->insert(innerHash);
    }
    if(!isExist)
      ObjPhantomInfo->insert(make_pair(TypeHash, PhantomSet));
  }
}

#ifdef HEX_LOG
extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __lookup_success_count(char VerifyResult) {
  IncVal(numCasting, 1);
  IncVal(numVerifiedCasting, 1);
  IncVal(numLookHit, 1);
  IncVal(numCastHit, 1);

  switch (VerifyResult) {
  case BADCAST:
    IncVal(numCastBadCast, 1);
    break;
  case SAFECASTUPCAST:
    IncVal(numCastNonBadCast, 1);
    break;
  case SAFECASTSAME:
    IncVal(numCastSame, 1);
    break;
  case FAILINFO:
    IncVal(numMissFindObj, 1);
    break;
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __obj_update_count(uint32_t objUpdateType, uint64_t vla) {
  switch (objUpdateType) {
  case STACKALLOC:
    IncVal(numStackUp, vla);
    break;
  case GLOBALALLOC:
    IncVal(numGloUp, vla);
    break;
  case HEAPALLOC:
  case REALLOC:
    IncVal(numHeapUp, vla);
    break;
  case PLACEMENTNEW:
    IncVal(numplacementNew, vla);
    break;
  case REINTERPRET:
    IncVal(numreinterpretCast, vla);
    break;
  }
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void __obj_remove_count(uint32_t objUpdateType, uint64_t vla) {
  switch (objUpdateType) {
  case STACKALLOC:
    IncVal(numStackRm, vla);
    break;
  }
}
#endif
