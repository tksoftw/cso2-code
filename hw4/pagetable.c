#include "config.h"
#include "mlpt.h"
#include <stddef.h>
#include <stdio.h>

size_t ptbr;

size_t translate(size_t va) {
    size_t page_size = 1 << POBITS;
    size_t po = va & (page_size - 1);
    size_t level = LEVELS;
    for (;;) {
        size_t vpn = va >> POBITS;

        size_t base = ptbr;
        size_t index = vpn;
        size_t scale = 8;

        size_t *pte_addr = (size_t*)(base + index * scale);
        size_t pte = *pte_addr;

        if (!(pte & 1)) { // check valid bit
            return -1;
        }

        size_t ppn = (pte >> POBITS);

        size_t pa = (ppn << POBITS) + po;

        if (level == 1) {
            return pa;
        }
    }
}