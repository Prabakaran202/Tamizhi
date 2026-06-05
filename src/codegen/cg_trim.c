#include "codegen_bridge.h"

void tamizhi_codegen_trim(char *str) {
    char *trim_p = str;
    while(isspace((unsigned char)*trim_p)) trim_p++;
    int len = strlen(trim_p);
    while(len > 0 && isspace((unsigned char)trim_p[len-1])) {
        trim_p[len-1] = '\0';
        len--;
    }
    memmove(str, trim_p, strlen(trim_p) + 1);
}
