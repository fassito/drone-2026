#pragma once

#include <stdio.h>
#include <math.h>
#include <string.h>

// Contadores globales del runner.
extern int g_checks;
extern int g_failures;

void test_section(const char *name);

#define CHECK(cond, msg) do { \
    g_checks++; \
    if (!(cond)) { g_failures++; printf("  FAIL: %s\n", msg); } \
    else { printf("  ok:   %s\n", msg); } \
} while (0)

#define CHECK_NEAR(a, b, tol, msg) do { \
    g_checks++; \
    double _va = (double)(a), _vb = (double)(b); \
    if (fabs(_va - _vb) > (tol)) { \
        g_failures++; \
        printf("  FAIL: %s (%.6f vs %.6f)\n", msg, _va, _vb); \
    } else { printf("  ok:   %s\n", msg); } \
} while (0)
