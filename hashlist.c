/**
* @file         : hashlist.c
* @author       : nokymali@163.com
* @create       : 2026-06-15 14:02
* @brief        : TODO
* Created by nokymali@163.com on 2026/6/15.
*/
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include "hashlist.h"

/**
 * Allocated New HashListTable Bucket.
 * @return HashListTableBucket
 */
static inline __attribute__((always_inline)) HashListTableBucket *HashListTableAllocNewBucket() {
    volatile uint8_t cnt = 0;
    HashListTableBucket *b = NULL;
    do {
        b = (HashListTableBucket *)calloc(1, sizeof(HashListTableBucket));
        if (NULL != b) {
            return b;
        }
        sched_yield();
    } while ((NULL == b) && (cnt++ < 3));
    fprintf(stderr, "Fail to Allocated HashListTable Bucket Three Times.\n");
    exit(EXIT_FAILURE);
}

/**
 * Create a Hash List Table, Size best to use prime value.
 * Must be specified the hash value calculate callback function.
 * Must be specified the bucket data compare callback function.
 * Must be specified the bucket data destroy callback function.
 * @param size HashListTable Size(It's best to use the prime value)
 * @param hash Calculate hash value callback function
 * @param cmp Bucket data compare callback function
 * @param destroy Bucket data destroy callback function
 * @return HashListTable
 */
HashListTable* HashListTableCreate(const uint32_t size, const BucketDataHashValue hash, const BucketDataCompare cmp, const BucketDataFree destroy) {
    if (0 == size) {
        fprintf(stderr, "Fail to create HashListTable. Prime Value is Zero.\n");
        return NULL;
    }

    if (NULL == hash) {
        fprintf(stderr, "Fail to create HashListTable. HashValue Calculate Callback is NULL.\n");
        return NULL;
    }

    if (NULL == cmp) {
        fprintf(stderr, "Fail to create HashListTable, Bucket Data Compare Callback is NULL.\n");
        return NULL;
    }

    if (NULL == destroy) {
        fprintf(stderr, "Fail to create HashListTable, Bucket Data Destroy Callback is NULL.\n");
        return NULL;
    }
    HashListTable *hlt = calloc(1, sizeof(HashListTable));
    if (NULL == hlt) {
        fprintf(stderr, "Fail to create HashListTable. Allocated HashListTable is NULL.\n");
        return NULL;
    }
    hlt->size  = size;
    hlt->bd_hash     = hash;
    hlt->bd_cmp      = cmp;
    hlt->bd_free     = destroy;
    hlt->array      = calloc(hlt->size, sizeof(HashListTableBucket *));
    if (NULL == hlt->array) {
        fprintf(stderr, "Fail to create HashListTable. Allocated Array Failure.\n");
        free(hlt);
        exit(EXIT_FAILURE);
    }

    hlt->count = 0;
    hlt->list_head   = NULL;
    hlt->list_tail   = NULL;
    pthread_spin_init(&hlt->spin, PTHREAD_PROCESS_PRIVATE);
    return hlt;
}

/**
 * Destroy HashListTable.
 * @param hlt HashListTable
 */
void HashListTableDestroy(HashListTable *hlt) {
    if ((NULL == hlt) || (NULL == hlt->array)) {
        return;
    }

    for (uint32_t i = 0; i < hlt->size; i++) {
        HashListTableBucket *bucket = hlt->array[i];
        while (NULL != bucket) {
            pthread_spin_lock(&hlt->spin);
            HashListTableBucket *next_bucket = bucket->bucket_next;
            hlt->bd_free(bucket->data);
            free(bucket);
            bucket = next_bucket;
            pthread_spin_unlock(&hlt->spin);
        }
    }
    free(hlt->array);
    pthread_spin_destroy(&hlt->spin);
    free(hlt);
    hlt = NULL;
}

/**
 * Add Data To HashListTable
 * @param hlt HashListTable
 * @param data Data
 * @return 0: add success, -1: add failure, 1: data already exist.
 */
int HashListTableAdd(HashListTable *hlt, void *data) {
    if (NULL == hlt || NULL == data) {
        return -1;
    }

    const uint32_t value = hlt->bd_hash(data);
    const uint32_t pos = value % hlt->size;
#if 0
    fprintf(stdout, "HashListTable Add: %p, Count: %"PRIu32", HashValue: %"PRIu32", Position: %"PRIu32"\n", ht, ht->total_count, value, pos);
#endif

    /**< 1. not exist. */
    if (NULL == hlt->array[pos]) {
        /**< 1.1 allocate new Bucket */
        HashListTableBucket *bucket = HashListTableAllocNewBucket();
        bucket->data        = data;
        bucket->bucket_next = NULL;
        bucket->list_next   = NULL;
        bucket->list_prev   = NULL;
        pthread_spin_lock(&hlt->spin);
        /**< 1.2 add data to bucket list head */
        hlt->array[pos] = bucket;

        /**< 1.3 add data to double list tail*/
        if (NULL == hlt->list_tail) {
            hlt->list_head = bucket;
            hlt->list_tail = bucket;
        } else {
            bucket->list_prev = hlt->list_tail;
            hlt->list_tail->list_next = bucket;
            hlt->list_tail = bucket;
        }

        /**< 1.4 statistic total count */
        hlt->count += 1;
        pthread_spin_unlock(&hlt->spin);
        return 0; // add success
    }

    /**< 2. position not empty */
    /**< 2.1 search data is exist */
    pthread_spin_lock(&hlt->spin);
    const HashListTableBucket *c = hlt->array[pos];
    while (NULL != c) {
        if (0 == hlt->bd_cmp(c->data, data)) {
            pthread_spin_unlock(&hlt->spin);
            return 1; // data is exist.
        }
        c = c->bucket_next;
    }
    pthread_spin_unlock(&hlt->spin);

    /**< 2.2 allocate new bucket */
    HashListTableBucket *bucket = HashListTableAllocNewBucket();
    bucket->data        = data;
    bucket->bucket_next = NULL;
    bucket->list_next   = NULL;
    bucket->list_prev   = NULL;
    pthread_spin_lock(&hlt->spin);
    /**< 2.3 add data to bucket list head */
    bucket->bucket_next = hlt->array[pos];
    hlt->array[pos] = bucket;

    /**< 2.4 add data to double list tail*/
    if (NULL == hlt->list_tail) {
        hlt->list_head = bucket;
        hlt->list_tail = bucket;
    } else {
        bucket->list_prev = hlt->list_tail;
        hlt->list_tail->list_next = bucket;
        hlt->list_tail = bucket;
    }
    /**< 2.5 statistic total count */
    hlt->count += 1;
    pthread_spin_unlock(&hlt->spin);
    return 0; // add success
}

/**
 *
 * @param hlt HashListTable
 * @param data Data
 * @return -1: failure, 0: success
 */
int HashListTableRemove(HashListTable *hlt, void *data) {
    if (NULL == hlt || NULL == data) {
        return -1;
    }
    const uint32_t value = hlt->bd_hash(data);
    const uint32_t pos = value % hlt->size;
#if 0
    fprintf(stdout, "HashListTable Remove: %p, Count: %"PRIu32", HashValue: %"PRIu32", Position: %"PRIu32"\n", ht, ht->total_count, value, pos);
#endif

    pthread_spin_lock(&hlt->spin);
    if (NULL == hlt->array[pos]) {
        pthread_spin_unlock(&hlt->spin);
        return 0;
    }
    if (NULL == hlt->array[pos]->bucket_next) {
        HashListTableBucket *curr = hlt->array[pos];
        if (0 == hlt->bd_cmp(curr->data, data)) {
            if (NULL == curr->list_prev) {
                hlt->list_head = curr->list_next;
            } else {
                curr->list_prev->list_next = curr->list_next;
            }

            if (NULL == curr->list_next) {
                hlt->list_tail = curr->list_prev;
            } else {
                curr->list_next->list_prev = curr->list_prev;
            }

            hlt->bd_free(curr->data);
            free(curr);
            hlt->array[pos] = NULL;

            if (hlt->count > 0) {
                hlt->count -= 1;
            }
            pthread_spin_unlock(&hlt->spin);
            return 0;
        }
        pthread_spin_unlock(&hlt->spin);
        return -1;
    }

    HashListTableBucket *prev = NULL;
    HashListTableBucket *curr = hlt->array[pos];
    do {
        if (0 == hlt->bd_cmp(curr->data, data)) {
            /**< Case1: Current Node is Double List Head. */
            if (NULL == curr->list_prev) {
                hlt->list_head = curr->list_next;
            } else {
                curr->list_prev->list_next = curr->list_next;
            }
            /**< Case2: Current Node is Dobule List Tail. */
            if (NULL == curr->list_next) {
                hlt->list_tail = curr->list_prev;
            } else {
                curr->list_next->list_prev = curr->list_prev;
            }

            /**< Case3: Bucket Single List */
            if (NULL == prev) {
                hlt->array[pos] = curr->bucket_next;
            } else {
                prev->bucket_next = curr->bucket_next;
            }

            /**< Free Current Node's Data */
            hlt->bd_free(curr->data);

            /**< Free Current Node */
            free(curr);
            /**< All the node's count */
            if (hlt->count > 0) {
                hlt->count -= 1;
            }
            pthread_spin_unlock(&hlt->spin);
            return 0;
        }
        prev = curr;
        curr = curr->bucket_next;
    } while (NULL != curr);
    pthread_spin_unlock(&hlt->spin);
    return -1;
}

/**
 * Lookup The Bucket Data
 * @param hlt HashListTable
 * @param data Data
 * @return Founded Bucket Data
 */
void *HashListTableLookup(HashListTable *hlt, void *data) {
    if (NULL == hlt || NULL == data) {
        return NULL;
    }

    const uint32_t value = hlt->bd_hash(data);
    const uint32_t pos = value % hlt->size;

    pthread_spin_lock(&hlt->spin);
    if (NULL == hlt->array[pos]) {
        pthread_spin_unlock(&hlt->spin);
        return NULL;
    }
    const HashListTableBucket *curr = hlt->array[pos];
    do {
        if (0 == hlt->bd_cmp(curr->data, data)) {
            pthread_spin_unlock(&hlt->spin);
            return curr->data;
        }
        curr = curr->bucket_next;
    } while (NULL != curr);
    pthread_spin_unlock(&hlt->spin);
    return NULL;
}

/**
 * Display HashListTable Information
 * @param hlt HashListTable
 */
void HashListTableInfo(HashListTable *hlt) {
    if (NULL == hlt) {
        return;
    }

    fprintf(stdout, "HashListTable Address:%p,Size:%"PRIu32",Count:%"PRIu32",BucketSize:%ld,ListHead:%p,ListTail:%p\n",
     hlt, hlt->size, hlt->count, sizeof(HashListTableBucket), hlt->list_head, hlt->list_tail);
}