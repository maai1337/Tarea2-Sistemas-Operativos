#include <stdio.h>
#include <stdint.h>

#define PAGE_BITS 12
#define MASK ((1 << PAGE_BITS) - 1)

void descomponer(unsigned int dv, unsigned int *nvp, unsigned int *offset) {
    *offset = dv & MASK;
    *nvp = dv >> PAGE_BITS;
}

int main() {
    unsigned int dv = 0xABCDEF;
    unsigned int nvp, offset;

    descomponer(dv, &nvp, &offset);

    printf("nvp = %X\n", nvp);
    printf("offset = %X\n", offset);

    return 0;
}
