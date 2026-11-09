#include "test_util.h"

int g_checks = 0;
int g_failures = 0;

void test_section(const char *name) {
    printf("\n== %s ==\n", name);
}
