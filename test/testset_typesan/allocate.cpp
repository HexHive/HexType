#include <stdlib.h>

#include "types.h"

extern BaseType* getbaseptr(AllocType *allocptr);
extern void checkcast(BaseType *ptr);

#ifdef ALLOC_ARGUMENT
void intermediate(AllocType byvalue) {
    if (&byvalue < (void*)0x1) {
        exit(-1);
    }
    checkcast(getbaseptr(&byvalue));
}
void allocate(int count) {
    AllocType stack;
    if (count == 1337) {
        exit(-1);
    }
    intermediate(stack);
}
#elif defined(ALLOC_GLOBAL)
AllocType global;
void allocate(int count) {
    AllocType *ptr = &global;
    checkcast(getbaseptr(ptr));
}
#elif defined(ALLOC_GLOBAL_ARRAY)
AllocType global[10];
void allocate(int count) {
#ifndef DO_PASSING
    {
        AllocType *ptr = &global[1];
#else
    for (int i = 0; i < 10; ++i) {
        AllocType *ptr = &global[i];
#endif
        checkcast(getbaseptr(ptr));
    }
}
#elif defined(ALLOC_GLOBAL_ARRAY_DEEP)
AllocType global[10][10];
void allocate(int count) {
#ifndef DO_PASSING
    {
        AllocType *ptr = &global[1][1];
#else
    for (int i = 0; i < 100; ++i) {
        AllocType *ptr = &global[i % 10][i / 10];
#endif
        checkcast(getbaseptr(ptr));
    }
}
#else
void allocate(int count) {
#ifdef ALLOC_STACK
    AllocType stack;
    if (count == 1337) {
        exit(-1);
    }
    {
        AllocType *ptr = &stack;
#endif
#ifdef ALLOC_STACK_ARRAY
    AllocType stack[10];
    if (count == 1337) {
        exit(-1);
    }
#ifndef DO_PASSING
    {
        AllocType *ptr = &(stack[1]);
#else
    for(int i = 0; i < 10; ++i) {
        AllocType *ptr = &(stack[i]);
#endif
#endif
#ifdef ALLOC_STACK_ARRAY_DEEP
    AllocType stack[3][3];
    if (count == 1337) {
        exit(-1);
    }
#ifndef DO_PASSING
    {
        AllocType *ptr = &(stack[1][1]);
#else
    for(int i = 0; i < 9; ++i) {
        AllocType *ptr = &(stack[i % 3][i / 3]);
#endif
#endif
#ifdef ALLOC_MALLOC
    {
        AllocType *ptr = (AllocType*)malloc(sizeof(AllocType));
#endif
#ifdef ALLOC_MALLOC_ARRAY
    AllocType *heap = (AllocType*)malloc(sizeof(AllocType) * 10);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < 10; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#ifdef ALLOC_MALLOC_VLA
    AllocType *heap = (AllocType*)malloc(sizeof(AllocType) * count);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < count; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#ifdef ALLOC_CALLOC_ARRAY
    AllocType *heap = (AllocType*)calloc(sizeof(AllocType), 10);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < 10; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#ifdef ALLOC_CALLOC_VLA
    AllocType *heap = (AllocType*)calloc(sizeof(AllocType), count);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < count; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#ifdef ALLOC_REALLOC
    void *tmp = malloc(8);
    {
        AllocType *ptr = (AllocType*)realloc(tmp, sizeof(AllocType));
#endif
#ifdef ALLOC_REALLOC_ARRAY
    void *tmp = malloc(8);
    AllocType *heap = (AllocType*)realloc(tmp, sizeof(AllocType) * 10);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < 10; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#ifdef ALLOC_REALLOC_VLA
    void *tmp = malloc(8);
    AllocType *heap = (AllocType*)realloc(tmp, sizeof(AllocType) * count);
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < count; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#if defined(ALLOC_NEW) || defined(ALLOC_OVERLOADED_NEW)
    {
        AllocType *ptr = new AllocType();
#endif
#if defined(ALLOC_NEW_ARRAY) || defined(ALLOC_OVERLOADED_NEW_ARRAY)
    AllocType *heap = new AllocType[10];
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < 10; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
#if defined(ALLOC_NEW_VLA) || defined(ALLOC_OVERLOADED_NEW_VLA)
    AllocType *heap = new AllocType[count];
#ifndef DO_PASSING
    {
        AllocType *ptr = &(heap[1]);
#else
    for(int i = 0; i < count; ++i) {
        AllocType *ptr = &(heap[i]);
#endif
#endif
        checkcast(getbaseptr(ptr));
    }
}
#endif 
