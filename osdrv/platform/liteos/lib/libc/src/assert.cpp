#include "assert.h"
#include "los_hwi.h"
#include "los_printf.h"

extern "C" void __assert2(const char* file, int line, const char* function, const char* failed_expression) {
    (VOID)LOS_IntLock();
    PRINT_ERR("%s:%d: %s: assertion \"%s\" failed", file, line, function, failed_expression);
    while (1)
        ;
}

extern "C"  void __assert (const char *assertion, int line, const char *file)
{
    (VOID)LOS_IntLock();
    PRINT_ERR("__assert error: %s, %s, %d\n", assertion, file, line);
    while(1);
}