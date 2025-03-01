#ifndef DICTARRAY_H_
#define DICTARRAY_H_

#include "dict.h"
#include "adlist.h"

typedef struct _kvstore kvstore;
typedef struct _kvstoreIterator kvstoreIterator;

typedef int (kvstoreScanShouldSkipDict)(dict *d);
typedef int (kvstoreExpandShouldSkipDictIndex)(int didx);

#define KVSTORE_ALLOCATE_DICTS_ON_DEMAND (1<<0)
#define KVSTORE_FREE_EMPTY_DICTS (1<<1)
kvstore *kvstoreCreate(dictType *type, int num_dicts_bits, int flags);
void kvstoreEmpty(kvstore *kvs, void(callback)(dict*));
void kvstoreRelease(kvstore *kvs);
unsigned long long kvstoreSize(kvstore *kvs);
unsigned long kvstoreBuckets(kvstore *kvs);
size_t kvstoreMemUsage(kvstore *kvs);
unsigned long long kvstoreScan(kvstore *kvs, unsigned long long cursor,
                               int onlydidx, dictScanFunction *scan_cb,
                               kvstoreScanShouldSkipDict *skip_cb,
                               void *privdata);
int kvstoreExpand(kvstore *kvs, uint64_t newsize, int try_expand, kvstoreExpandShouldSkipDictIndex *skip_cb);
int kvstoreGetFairRandomDictIndex(kvstore *kvs);
void kvstoreGetStats(kvstore *kvs, char *buf, size_t bufsize, int full);

int kvstoreFindDictIndexByKeyIndex(kvstore *kvs, unsigned long target);
int kvstoreGetNextNonEmptyDictIndex(kvstore *kvs, int didx);
int kvstoreNumNonEmptyDicts(kvstore *kvs);
int kvstoreNumDicts(kvstore *kvs);
uint64_t kvstoreGetHash(kvstore *kvs, const void *key);

/* kvstore iterator specific functions */
kvstoreIterator *kvstoreIteratorInit(kvstore *kvs);
void kvstoreIteratorRelease(kvstoreIterator *kvs_it);
dict *kvstoreIteratorNextDict(kvstoreIterator *kvs_it);
int kvstoreIteratorGetCurrentDictIndex(kvstoreIterator *kvs_it);
dictEntry *kvstoreIteratorNext(kvstoreIterator *kvs_it);

/* Rehashing */
void kvstoreTryResizeDicts(kvstore *kvs, int limit);
uint64_t kvstoreIncrementallyRehash(kvstore *kvs, uint64_t threshold_ms);

/* Specific dict access by dict-index */
unsigned long kvstoreDictSize(kvstore *kvs, int didx);
dictIterator *kvstoreDictGetIterator(kvstore *kvs, int didx);
dictIterator *kvstoreDictGetSafeIterator(kvstore *kvs, int didx);
dictEntry *kvstoreDictGetRandomKey(kvstore *kvs, int didx);
dictEntry *kvstoreDictGetFairRandomKey(kvstore *kvs, int didx);
dictEntry *kvstoreDictFindEntryByPtrAndHash(kvstore *kvs, int didx, const void *oldptr, uint64_t hash);
unsigned int kvstoreDictGetSomeKeys(kvstore *kvs, int didx, dictEntry **des, unsigned int count);
int kvstoreDictExpand(kvstore *kvs, int didx, unsigned long size);
unsigned long kvstoreDictScanDefrag(kvstore *kvs, int didx, unsigned long v, dictScanFunction *fn, dictDefragFunctions *defragfns, void *privdata);
void *kvstoreDictFetchValue(kvstore *kvs, int didx, const void *key);
dictEntry *kvstoreDictFind(kvstore *kvs, int didx, void *key);
dictEntry *kvstoreDictAddRaw(kvstore *kvs, int didx, void *key, dictEntry **existing);
void kvstoreDictSetKey(kvstore *kvs, int didx, dictEntry* de, void *key);
void kvstoreDictSetVal(kvstore *kvs, int didx, dictEntry *de, void *val);
dictEntry *kvstoreDictTwoPhaseUnlinkFind(kvstore *kvs, int didx, const void *key, dictEntry ***plink, int *table_index);
void kvstoreDictTwoPhaseUnlinkFree(kvstore *kvs, int didx, dictEntry *he, dictEntry **plink, int table_index);
int kvstoreDictDelete(kvstore *kvs, int didx, const void *key);

#endif /* DICTARRAY_H_ */
