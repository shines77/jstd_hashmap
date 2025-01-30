#include <stdio.h>
#include <stdlib.h>

int main() {
    #ifdef __x86_64__
    printf("x86_64\n");
    #elif defined(__i386__)
    printf("i386\n");
    #elif defined(__arm__)
    printf("arm\n");
    #elif defined(__aarch64__)
    printf("aarch64\n");
    #elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    printf("powerpc\n");
    #elif defined(__mips__)
    printf("mips\n");
    #elif defined(__s390__)
    printf("s390\n");
    #elif defined(__sparc__)
    printf("sparc\n");
    #else
    printf("unknown\n");
    #endif

    return 0;
}
