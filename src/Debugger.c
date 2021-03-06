#include "include/Debugger.h"


void kc_log_err(const char* error_msg, const char* offendingLine, unsigned long long lineNum) {
    printf("\033[93m%s\nLine %d\n", error_msg, lineNum);

    if (strcmp(offendingLine, "") != 0) {
        printf("\033[91m%s\n", offendingLine);

        for (int i = 0; i < strlen(offendingLine); ++i) {
            printf("^");
        }
    }
}
