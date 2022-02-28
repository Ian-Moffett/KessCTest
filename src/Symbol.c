#include "include/Symbol.h"

unsigned long long symb_tbl_hash(const char* key, const int MAX_SIZE) {
    unsigned int sum = 0;

    for (int i = 0; i < strlen(key); ++i) {
        sum += key[i];
    }

    if (MAX_SIZE == 0) {
        return -1;
    }

    return sum % MAX_SIZE;
}
