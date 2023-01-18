#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct HashTableNode {
    void *key;
    void *value;
    struct HashTableNode *next;
} HashTableNode;

HashTableNode *make_hash_node(void *key, void *value, HashTableNode *next);
void delete_hash_node(HashTableNode *node);

typedef struct HashTable {
    int size;
    int count;
    int (*hash)(void *key);
    int (*compare)(void *key1, void *key2);
    void (*destroy)(void *data);
    HashTableNode **hashes;
} HashTable;

HashTable *make_hash_table(int size, int (*hash)(void *key), int (*compare)(void *key1, void *key2), void (*destroy)(void *data));
void delete_hash_table(HashTable *hash_table);

void *hash_table_get(HashTable *hash_table, void *key);
int hash_table_set(HashTable *hash_table, void *key, void *value);
int hash_table_delete(HashTable *hash_table, void *key);

#endif