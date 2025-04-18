// hash_table.c
#ifndef HTTPD_HTABLE
#define HTTPD_HTABLE

#include <string.h>
#include <math.h>
#include "prime.c"
#include <stdio.h>

#define HT_INITIAL_BASE_SIZE 17
#define HT_PRIME_1 163
#define HT_PRIME_2 157

typedef struct {
    char* key;
    char* value;
} ht_item;

typedef struct {
    int size;
    int count;
    ht_item** items;
    int base_size;
} ht_htable;

ht_item HT_DELETED_ITEM = {NULL, NULL};

void ht_resize(ht_htable* ht, const int base_size);
ht_htable* ht_new(void);
void ht_del_htable(ht_htable* ht);
void ht_insert(ht_htable* ht, const char* key, const char* value);
char* ht_search(ht_htable* ht, const char* key);
void ht_delete(ht_htable* ht, const char* key);

ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

void ht_del_item(ht_item* i) {
    free(i->key);
    free(i->value);
    free(i);
}

void ht_clear(ht_htable* ht) {
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL) {
            if (item != &HT_DELETED_ITEM) {
                ht_del_item(item);
            }
            ht->items[i] = NULL;
        }
    }
    ht->count = 0;
}

ht_htable* ht_new_sized(const int base_size) {
    ht_htable* ht = malloc(sizeof(ht_htable));
    ht->base_size = base_size;

    ht->size = next_prime(ht->base_size);

    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
    return ht;
}


ht_htable* ht_new(void) {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

void ht_del_htable(ht_htable* ht) {
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_del_item(item);
        }
    }
    free(ht->items);
    free(ht);
}

size_t ht_hash(const char* s, const int a, const int m) {
    size_t hash = 0;
    const int len_s = strlen(s);
    for (int i = 0; i < len_s; i++)
        hash += pow(a, len_s - (i+1)) * s[i];
    hash = hash % m;
    return hash;
}

size_t ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    const size_t hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    size_t hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    if (hash_b % num_buckets == 0)
        hash_b = 1;
    return (hash_a + (attempt * hash_b)) % num_buckets;
}

void ht_resize_up(ht_htable* ht) {
    int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}


void ht_resize_down(ht_htable* ht) {
    int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

void ht_resize(ht_htable* ht, int base_size) {
    if (base_size < HT_INITIAL_BASE_SIZE)
        base_size = HT_INITIAL_BASE_SIZE;
    ht_htable* new_ht = ht_new_sized(base_size);
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    // To delete new_ht, we give it ht's size and items 
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    ht_del_htable(new_ht);
}

void ht_insert(ht_htable* ht, const char* key, const char* value) {
    const int load = ht->count * 100 / ht->size;
    if (load > 70)
        ht_resize_up(ht);
    ht_item* item = ht_new_item(key, value);
    size_t index = ht_get_hash(item->key, ht->size, 0);
    ht_item* cur_item = ht->items[index];
    int i = 1;
    while (cur_item != NULL) {
        if (cur_item != &HT_DELETED_ITEM) {
            if (strcmp(cur_item->key, key) == 0) {
                ht_del_item(cur_item);
                ht->items[index] = item;
                return;
            }
        }
        index = ht_get_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }
    ht->items[index] = item;
    ht->count++;
}

char* ht_search(ht_htable* ht, const char* key) {
    size_t index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
            return item->value;
        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    } 
    return NULL;
}

void ht_delete(ht_htable* ht, const char* key) {
    const int load = ht->count * 100 / ht->size;
    if (load < 30)
        ht_resize_down(ht);
    size_t index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                ht_del_item(item);
                ht->items[index] = &HT_DELETED_ITEM;
                ht->count--;
                return;
            }
        }
        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    } 
}

void ht_print_table(ht_htable* table) {
    for (int i = 0; i < table->size; i++) {
        ht_item* item = table->items[i];
        if (item == NULL)
            printf("(NULL)->(NULL)\n");
        else 
            printf("%s->%s\n", item->key, item->value);
    }
}
#endif
