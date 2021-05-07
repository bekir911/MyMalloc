#pragma once
#ifndef _MYMALLOC_H
#define _MYMALLOC_H

#include <stdio.h>
#include <unistd.h>

typedef struct info {
    unsigned int isfree;    /* if zero, it is NOT free */
    unsigned int size;      /* 4 byte */
}Info;                      /* 4 + 4 byte */

typedef struct block {
    Info info;              /* 8 byte */
    struct block* next;     /* 8 byte */
    struct block* prev;     /* 8 byte */    
    char data[0];           /* 0 byte */
}Block;                     /* 8 + 8 + 8 + 0 byte */

typedef enum {bestFit, worstFit, firstFit} Strategy;
Strategy strategy = firstFit;

static Block* free_list = NULL;
static Block* heap_start = NULL;
int is_first = 1;               /* if zero, NOT first call */
const int B = sizeof(Info);     /* size of boundary tag */
const int M = sizeof(Block);    /* size of metadata */
const int SIZE = 1024;          /* max size */
int isNotMergedNext = 0;        /* if one, NOT merged with next block */
int isNotMergedPrev = 0;        /* if one, NOT merged with previous block */

void* mymalloc(size_t size);
void* myfree(void* ptr);
void* bestFitStrategy(size_t size);
void* worstFitStrategy(size_t size);
void* firstFitStrategy(size_t size);
Block* split(Block* b, size_t size);
Block* mergeNext(Block* b);
Block* mergePrev(Block* b);
void addFreeList(Block* b);
void removeFreeList(Block* b);
void printHeap();

#endif  /* mymalloc.h included */