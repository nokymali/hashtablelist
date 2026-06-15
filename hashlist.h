/**
* @file         : hashlist.h
* @author       : nokymali@163.com
* @create       : 2026-06-15 14:02
* @brief        : TODO
* Created by nokymali@163.com on 2026/6/15.
*/


#ifndef HASH_LIST_H
#define HASH_LIST_H
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*BucketDataHashValue)(void *);
typedef int (*BucketDataCompare)(void *, void *);
typedef void (*BucketDataFree)(void *);

typedef struct HashListTableBucket_ {
    void                        *data;
    struct HashListTableBucket_ *bucket_next;
    struct HashListTableBucket_ *list_prev;
    struct HashListTableBucket_ *list_next;
} HashListTableBucket;


typedef struct HashListTable_ {
    HashListTableBucket **array;
    HashListTableBucket *list_head;
    HashListTableBucket *list_tail;
    BucketDataHashValue bd_hash;
    BucketDataCompare   bd_cmp;
    BucketDataFree      bd_free;
    pthread_spinlock_t  spin;
    uint32_t            size;
    uint32_t            count;
} HashListTable;

HashListTable* HashListTableCreate(uint32_t, BucketDataHashValue, BucketDataCompare, BucketDataFree);
void HashListTableDestroy(HashListTable *);
int HashListTableAdd(HashListTable *, void *);
int HashListTableRemove(HashListTable *, void *);
void *HashListTableLookup(HashListTable *, void *);
void HashListTableInfo(HashListTable *);

#ifdef __cplusplus
}
#endif
#endif //HASH_LIST_H
