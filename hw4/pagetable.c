#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "mlpt.h"

#define VALID_BIT 1ULL
#define PT_BYTES_SHIFT 3
#define PAGE_SIZE (1ULL << POBITS)

size_t ptbr = 0;

static inline size_t *pte_addr_at_level(size_t full_va, size_t level, size_t base) {
    size_t full_vpn = full_va >> POBITS;            // full_vpn is divided per-level into vpn_parts
    size_t vpn_part_bits = POBITS - PT_BYTES_SHIFT; // bits per vpn_part
    size_t vpn_part_mask = (1ULL << vpn_part_bits) - 1;

    size_t shift = (level - 1) * vpn_part_bits;                    // select this level's vpn_part (top-down)
    size_t index = (full_vpn & (vpn_part_mask << shift)) >> shift; // extract this level's vpn_part and shift to LSB
    size_t scale = (1ULL << PT_BYTES_SHIFT);                       // 8 bytes (PTE size)

    size_t *pte_addr = (size_t *)(base + index * scale);
    return pte_addr;
}

size_t translate(size_t va) {
    if (ptbr == 0) {
        return (size_t)-1;
    }

    size_t base = ptbr;
    for (size_t level = LEVELS; level > 0; --level) {
        size_t *pte_addr = pte_addr_at_level(va, level, base);
        size_t pte = *pte_addr;

        if (!(pte & VALID_BIT)) {
            return (size_t)-1;
        }

        size_t ppn = pte >> POBITS; // extract physical page number (ppn)
        base = ppn << POBITS;       // convert ppn to base address for next level
    }

    size_t page_offset = va & (PAGE_SIZE - 1);
    return base + page_offset;
}

static int allocate_random_page(void **ptr) {
    int err = posix_memalign(ptr, PAGE_SIZE, PAGE_SIZE);
    if (err != 0) {
        return err;
    }
    memset(*ptr, 0, PAGE_SIZE);
    return 0;
}

int allocate_page(size_t start_va) {
    if ((start_va % PAGE_SIZE) != 0) { // start_va must be at the start of a page
        return -1;
    }
    if (translate(start_va) != (size_t)-1) { // memory already allocated
        return 0;
    }

    if (ptbr == 0) { // allocate ptbr if not already
        void *root_ptr = NULL;
        int err = allocate_random_page(&root_ptr);
        if (err != 0) {
            return -1;
        }
        ptbr = (size_t)root_ptr;
    }

    size_t base = ptbr;
    for (size_t level = LEVELS; level > 0; --level) {
        size_t *pte_addr = pte_addr_at_level(start_va, level, base);
        size_t pte = *pte_addr;

        if (!(pte & VALID_BIT)) {
            void *new_page;
            int err = allocate_random_page(&new_page);
            if (err != 0) {
                return -1;
            }
            pte = (size_t)new_page | VALID_BIT;
            *pte_addr = pte;
        }

        if (level == 1) { // no more work to do
            return 1;
        }

        size_t ppn = pte >> POBITS; // extract physical page number (ppn)
        base = ppn << POBITS;       // convert ppn to base address for next level
    }
    return -1;
}