
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint64_t num_buckets;
    void *data;
} hashtable_t;

hashtable_t *hashtable_create(uint64_t const num_buckets) {
    hashtable_t *const table = malloc(sizeof(hashtable_t));
    void *const data =  calloc(num_buckets, 8);
    table->num_buckets = num_buckets;
    table->data = data;
    return table;
}

void hashtable_destroy(hashtable_t *const table) {
    free(table->data);
    free(table);
}

static uint64_t hash(const char *const key) {
    uint64_t v = 0;
    size_t i = 0;
    while (key[i] != '\0') {
        v += key[i];
        i++;
    }
    return v;
}

void hashtable_set(const hashtable_t *const table, const char *const key, const void *const value) {
    uint64_t const bucket = hash(key) % table->num_buckets;
}



int main() {
    hashtable_t *const table = hashtable_create(10);
    hashtable_set(table, "asdf", "dddd");
    hashtable_destroy(table);

    printf("%lld\n", hash("saaa"));
}