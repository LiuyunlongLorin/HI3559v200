#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include "limits.h"

char *osal_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}
char *osal_strncpy(char *dest, const char *src, int count)
{
    return strncpy(dest, src, count);
}
int osal_strlcpy(char *dest, const char *src, int size)
{
    return strlcpy(dest, src, size);
}
char *osal_strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}
char *osal_strncat(char *dest, const char *src, int count)
{
    return strncat(dest, src, count);
}
int osal_strlcat(char *dest, const char *src, int count)
{
    return strlcat(dest, src, count);
}

int osal_strcmp(const char *cs, const char *ct)
{
    return strcmp(cs, ct);
}
int osal_strncmp(const char *cs, const char *ct, int count)
{
    return strncmp(cs, ct, count);
}
int osal_strnicmp(const char *s1, const char *s2, int len)
{
    return strnicmp(s1, s2, len);
}
int osal_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}
int osal_strncasecmp(const char *s1, const char *s2, int n)
{
    return strncasecmp(s1, s2, n);
}
char *osal_strchr(const char *s, int c)
{
    return strchr(s, c);
}
char *osal_strnchr(const char *s, int count, int c)
{
    return strnchr(s, count, c);
}

char *osal_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

char *osal_strstr(const char *s1, const char *s2)
{
    return strstr(s1, s2);
}
char *osal_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}
int osal_strlen(const char *s)
{
    return strlen(s);
}
int osal_strnlen(const char *s, int count)
{
    return strnlen(s, count);
}
char *osal_strpbrk(const char *cs, const char *ct)
{
    return strpbrk(cs, ct);
}
char *osal_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}
int osal_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}
int osal_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}
void *osal_memset(void *str, int c, int count)
{
    return memset(str, c, count);
}
void *osal_memcpy(void *dest, const void *src, int count)
{
    return memcpy(dest, src, count);
}
void *osal_memmove(void *dest, const void *src, int count)
{
    return memmove(dest, src, count);
}
void *osal_memscan(void *addr, int c, int size)
{
    return NULL;
}
int osal_memcmp(const void *cs, const void *ct, int count)
{
    return memcmp(cs, ct, count);
}
void *osal_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}
void *osal_memchr_inv(const void *start, int c, int bytes)
{
    return NULL;
}
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base)
{
    return 0;
}
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
    return strtoul(cp, endp, base);
}
long osal_strtol(const char *cp, char **endp, unsigned int base)
{
    return simple_strtol(cp, endp, base);
}
long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
    return 0;
}
int osal_snprintf(char *buf, int size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return i;
}
int osal_scnprintf(char *buf, int size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vscnprintf(buf, size, fmt, args);
    va_end(args);

    return i;
}

int osal_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, INT_MAX, fmt, args);
    va_end(args);

    return i;
}
int osal_sscanf(const char *buf, const char *fmt, ...)
{
    // do not support in liteos
    dprintf("Do not support in liteos\n");

    return -1;
}

int osal_vsnprintf(char *str, int size, const char *fmt, osal_va_list args)
{
    return vsnprintf(str, size, fmt, args);
}


