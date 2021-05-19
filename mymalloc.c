/**
 * J3ekir
 * malloc and free implementation
*/

#include "mymalloc.h"

/**
 * allocates memory with sbrk
*/
void* mymalloc(size_t size) {
    /* if size isn't enough or less than zero */
    if (size > SIZE - M - B && size <= 0) {
        return NULL;
    }

    /* if it's first call, allocated memory and adds it to the free list */
    if (is_first) {
        heap_start = sbrk(SIZE);

        /* if sbrk fails */
        if ((void*)-1 == heap_start) {
            perror("sbrk error");
            return NULL;
        }

        is_first = 0;
        free_list = heap_start;
        free_list->next = NULL;
        free_list->prev = NULL;
        heap_start->info.size = SIZE - M - B;
        heap_start->info.isfree = 1;
        Info* info = (Info*)((char*)heap_start + heap_start->info.size + M);
        info->size = heap_start->info.size;
        info->isfree = 1;
    }

    /* if sbrk is full */
    if (NULL == free_list) {
        return NULL;
    }

    /* selects strategy */
    switch(strategy) {
        case bestFit:   return bestFitStrategy(size);
        case worstFit:  return worstFitStrategy(size);
        case firstFit:  return firstFitStrategy(size);
    }

    return NULL;
}

/**
 * adds taken memory to the free list
*/
void* myfree(void* ptr) {
    /* if it's a null pointer */
    if (NULL == ptr) {
        printf("ptr is NULL\n");
        return NULL;
    }

    Block* b = (Block*)((char*)ptr - M);

    /* if it's already freed */
    if (b->info.isfree) {
        printf("b is already freed\n");
    }

    isNotMergedNext = 0;
    isNotMergedPrev = 0;
    b = mergeNext(b);
    b = mergePrev(b);

    /* if right and left blocks are free */
    if (isNotMergedNext && isNotMergedPrev) {
        addFreeList(b);
        b->info.isfree = 1;
        Info* bTag = (Info*)((char*)b + b->info.size + M);
        bTag->isfree = 1;
    }

    return NULL;
}

/**
 * allocates memory in order to best fit strategy
*/
void* bestFitStrategy(size_t size) {
    Block* b = free_list;
    Block* minBlock = free_list;
    size_t min = SIZE;

    while (b != NULL) {
        if (((size + 15) / 16) * 16 <= b->info.size && b->info.size < min) {
            min = b->info.size;
            minBlock = b;
        }

        b = (Block*)b->next;
    }

    if (NULL == minBlock) {
        return NULL;
    }

    Block* newBlock = split(minBlock, size);
    return newBlock->data;
}

/**
 * allocates memory in order to worst fit strategy
*/
void* worstFitStrategy(size_t size) {
    Block* b = free_list;
    Block* maxBlock = free_list;
    size_t max = SIZE;

    while (b != NULL) {
        if (((size + 15) / 16) * 16 <= b->info.size && b->info.size > max) {
            max = b->info.size;
            maxBlock = b;
        }

        b = (Block*)b->next;
    }

    if (NULL == maxBlock) {
        return NULL;
    }

    Block* newBlock = split(maxBlock, size);
    return newBlock->data;
}

/**
 * allocates memory in order to first fit strategy
*/
void* firstFitStrategy(size_t size) {
    Block* b = free_list;
    while (b != NULL && ((size + 15) / 16) * 16 > b->info.size) {
        b = (Block*)b->next;
    }

    if (NULL == b) {
        return NULL;
    }

    Block* newBlock = split(b, size);
    return newBlock->data;
}

/**
 * splits the given block by given size
*/
Block* split(Block* b, size_t size) {
    /* if remainder size is less than a byte  */
    if (b->info.size < ((size + 15) / 16) * 16 + 16 + M + B) {
        removeFreeList(b);
        b->info.isfree = 0;
        Info* bTag = (Info*)((char*)b + b->info.size + M);
        bTag->isfree = 0;
        return b;
    }

    size_t bSize = b->info.size;

    Block* b1 = b;
    b1->info.size = ((size + 15) / 16) * 16;
    b1->info.isfree = 0;
    Info* bTag1 = (Info*)((char*)b + b->info.size + M);
    bTag1->size = b1->info.size;
    bTag1->isfree = 0;
    removeFreeList(b1);

    Block* b2 = (Block*)((char*)b1 + b1->info.size + M + B);
    b2->info.size = bSize - b1->info.size - M - B;
    b2->info.isfree = 1;
    Info* bTag2 = (Info*)((char*)b2 + b2->info.size + M);
    bTag2->size = b2->info.size;
    bTag2->isfree = 1;
    addFreeList(b2);

    return b1;
}

/**
 * coaleces if the next block is free
*/
Block* mergeNext(Block* b) {
    Info* bTag = (Info*)((char*)b + b->info.size + M);

    /* if it's end of the heap */
    if ((size_t)heap_start + SIZE == (size_t)((char*)bTag + B)) {
        isNotMergedNext = 1;
        return b;
    }

    Block* right = (Block*)((char*)bTag + B);
    
    /* if right block is free */
    if (right->info.isfree) {
        Info* rightbTag = (Info*)((char*)right + right->info.size + M);
        right->info.isfree = 0;
        rightbTag->isfree = 0;
        removeFreeList(right);
        
        Block* newBlock = b;
        newBlock->info.size = b->info.size + right->info.size + M + B;
        newBlock->info.isfree = 1;
        Info* newInfo = (Info*)((char*)newBlock + newBlock->info.size + M);
        newInfo->size = newBlock->info.size;
        newInfo->isfree = 1;
        addFreeList(newBlock);
    }
    else {
        isNotMergedNext = 1;
    }

    return b;
}

/**
 * coaleces if the previous block is free
*/
Block* mergePrev(Block* b) {
    /* if it's start of the heap */
    if ((size_t)heap_start == (size_t)b) {
        isNotMergedPrev = 1;
        return b;
    }

    Info* bTag = (Info*)((char*)b + b->info.size + M);
    Info* leftbTag = (Info*)((char*)b - B);
    Block* left = (Block*)((char*)leftbTag - leftbTag->size - M);

    /* if left block is free */
    if (left->info.isfree) {
        b->info.isfree = 0;
        bTag->isfree = 0;
        removeFreeList(b);

        Block* newBlock = left;
        newBlock->info.size = left->info.size + b->info.size + M + B;
        newBlock->info.isfree = 1;
        Info* newInfo = (Info*)((char*)newBlock + newBlock->info.size + M);
        newInfo->size = newBlock->info.size;
        newInfo->isfree = 1;
        return newBlock;
    }

    isNotMergedPrev = 1;
    return b;
}

/**
 * adds given block to the free list
*/
void addFreeList(Block* b) {
    /* if free list is empty */
    if (NULL == free_list) {
        free_list = b;
        free_list->next = NULL;
        free_list->prev = NULL;
    }
    else {  /* if free list is not empty */
        b->next = free_list;
        free_list->prev = b;
        free_list = b;
    }
}

/**
 * removes given block from the free list
*/
void removeFreeList(Block* b) {
    /* if there's only one block in free list */
    if (NULL == b->prev && NULL == b->next) {
        free_list = NULL;
        return;
    }

    /* if it's start of the free list */
    if (NULL == b->prev) {
        Block* right = b->next;
        right->prev = NULL;
        b->next = NULL;
        free_list = right;
        return;
    }

    /* if it's end of the free list */
    if (NULL == b->next) {
        Block* left = b->prev;
        left->next = NULL;
        b->prev = NULL;
        return;
    }

    /* if it's between two blocks */
    Block* left = b->prev;
    Block* right = b->next;
    b->next = NULL;
    b->prev = NULL;
    left->next = right;
    right->prev = left;
}

/**
 * prints all blocks in heap
*/
void printHeap() {
    Block* b = heap_start;

    if (NULL == b) {
        return;
    }

    printf("Blocks:\n");

    while ((size_t)b < (size_t)heap_start + SIZE) {
        printf("Free: %d\n", b->info.isfree);
        printf("Size: %d\n", b->info.size);
        printf("-------------------------\n");
        b = (Block*)((char*)b + b->info.size + M + B);
    }
}

int main() {
    /* test case */
    int* a1 = (int*)mymalloc(600);  /* 608 */
    if (NULL == a1) {
        printf("a1 is NULL\n");
        return -1;
    }

    int* a2 = (int*)mymalloc(30);   /* 32 */
    if (NULL == a1) {
        printf("a1 is NULL\n");
        return -1;
    }

    int* a3 = (int*)mymalloc(25);   /* 32 */
    if (NULL == a3) {
        printf("a3 is NULL\n");
        return -1;
    }

    printHeap();

    myfree(a2);
    printHeap();
    myfree(a1);
    printHeap();
    myfree(a3);

    printHeap();

    return 0;
}
