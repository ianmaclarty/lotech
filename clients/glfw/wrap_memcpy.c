/* This is to remove a dependency on glibc 2.14 */

#include <string.h>

asm (".symver memcpy, memcpy@GLIBC_2.2.5");

void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
        return memcpy(dest, src, n);
}
