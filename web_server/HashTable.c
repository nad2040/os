#include "HashTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

HashTableNode *make_hash_node(void *key, void *value, HashTableNode *next) {
    HashTableNode *node = malloc(sizeof(HashTableNode));
    node->key = key;
    node->value = value;
    node->next = next;
    return node;
}
void delete_hash_node(HashTableNode *node) {
    free(node);
}

HashTable *make_hash_table(int size, int (*hash)(void *key), int (*compare)(void *key1, void *key2), void (*destroy)(void *key)) {
    HashTable *table = malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->hash = hash;
    table->compare = compare;
    table->destroy = destroy;
    table->hashes = malloc(sizeof(HashTableNode *) * size);
    for (int i = 0; i < size; i++) {
        table->hashes[i] = NULL;
    }
    return table;
}
void delete_hash_table(HashTable *hash_table) {
    for (int i = 0; i < hash_table->size; i++) {
        HashTableNode *node = hash_table->hashes[i];
        while (node != NULL) {
            HashTableNode *next = node->next;
            delete_hash_node(node);
            node = next;
        }
    }
    free(hash_table->hashes);
    free(hash_table);
}

void *hash_table_get(HashTable *hash_table, void *key) {
    int hash = hash_table->hash(key) % hash_table->size;
    HashTableNode *node = hash_table->hashes[hash];
    while (node != NULL) {
        if (hash_table->compare(key, node->key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}
int hash_table_set(HashTable *hash_table, void *key, void *value) {
    int hash = hash_table->hash(key) % hash_table->size;
    HashTableNode *node = hash_table->hashes[hash];
    while (node != NULL) {
        if (hash_table->compare(key, node->key) == 0) {
            node->value = value;
            return 0;
        }
        node = node->next;
    }
    hash_table->hashes[hash] = make_hash_node(key, value, hash_table->hashes[hash]);
    hash_table->count++;
    return 1;
}
int hash_table_delete(HashTable *hash_table, void *key) {
    int hash = hash_table->hash(key) % hash_table->size;
    HashTableNode *node = hash_table->hashes[hash];
    HashTableNode *prev = NULL;
    while (node != NULL) {
        if (hash_table->compare(key, node->key) == 0) {
            if (prev == NULL) {
                hash_table->hashes[hash] = node->next;
            } else {
                prev->next = node->next;
            }
            delete_hash_node(node);
            hash_table->count--;
            return 1;
        }
        prev = node;
        node = node->next;
    }
    return 0;
}