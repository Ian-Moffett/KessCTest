#include "include/Symbol.h"

unsigned long long hashmap_hash(const char* key, const int MAX_SIZE) {
    int sum = 0;

    for (int i = 0; i < strlen(key); ++i) {
        sum += key[i];
    }

    if (MAX_SIZE == 0) {
        return -1;
    }

    return sum % MAX_SIZE;
}
