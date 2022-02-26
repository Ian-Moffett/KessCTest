#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>


int main(int argc, char* argv[]) {
    char* srcFile = NULL;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            srcFile = argv[i];
            break;
        }
    }

    if (!(srcFile)) {
        printf("Usage: %s <optional args> <filename>", argv[0]);
        exit(0);
    }

    FILE* fp = fopen(srcFile, "r");

    fseek(fp, 0, SEEK_END);
    unsigned long long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)calloc(fsize, sizeof(char));

    fread(buffer, sizeof(char), fsize, fp);

    printf("%s\n", buffer);

    fclose(fp);
}
