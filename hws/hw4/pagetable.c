#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "mlpt.h"

#define VALID_BIT 1ULL
#define PT_BYTES_SHIFT 3
#define PAGE_SIZE (1ULL << POBITS)
#define PTE_SIZE (1ULL << PT_BYTES_SHIFT)
#define DATA_PAGE_INDEX 0

size_t ptbr = 0;

static inline size_t *pte_addr_at_level(size_t full_va, size_t level, size_t base) {
    size_t full_vpn = full_va >> POBITS;            // full_vpn is divided per-level into vpn_parts
    size_t vpn_part_bits = POBITS - PT_BYTES_SHIFT; // bits per vpn_part
    size_t vpn_part_mask = (1ULL << vpn_part_bits) - 1;

    size_t shift = (level - 1) * vpn_part_bits;                    // select this level's vpn_part (top-down)
    size_t index = (full_vpn & (vpn_part_mask << shift)) >> shift; // extract this level's vpn_part and shift to LSB
    size_t scale = PTE_SIZE;

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

static size_t count_valid_ptes(size_t base) {
    size_t count = 0;
    size_t *pagetable = (size_t *)base;
    for (size_t i = 0; i < PAGE_SIZE / PTE_SIZE; ++i) {
        if (pagetable[i] & VALID_BIT) {
            ++count;
        }
    }
    return count;
}

int deallocate_page(size_t start_va) {
    if ((start_va % PAGE_SIZE) != 0) { // start_va must be at the start of a page
        return -1;
    }

    if (translate(start_va) == (size_t)-1) { // memory already deallocated
        return 0;
    }

    size_t pt_bases[LEVELS + 1]; // make level 0 = data page, 1 = pt1, ...
    size_t *pte_addrs[LEVELS];   // just LEVEL ptes because they go between

    size_t base = ptbr;
    pt_bases[LEVELS] = base; // set initial base because we don't do it in the loop
                             // (it makes indexing easier)

    // walk down (and save path)
    for (size_t level = LEVELS; level > 0; --level) {
        size_t *pte_addr = pte_addr_at_level(start_va, level, base);
        pte_addrs[level - 1] = pte_addr;

        base = (*pte_addr) & (~(PAGE_SIZE - 1)); // extract top bits of PTE == PPN
        pt_bases[level - 1] = base;
    }

    // deallocate data page
    *pte_addrs[DATA_PAGE_INDEX] &= ~VALID_BIT; // set parent pagetable invalid
    free((void *)pt_bases[DATA_PAGE_INDEX]);

    // walk up (from path)
    for (size_t level = 1; level < LEVELS; ++level) {
        base = pt_bases[level];
        if (count_valid_ptes(base) > 0) {
            return 1;
        }
        *pte_addrs[level] &= ~VALID_BIT; // set parent pagetable invalid
        free((void *)base);
    }

    // free root if contains no entries
    if (count_valid_ptes(ptbr) == 0) {
        free((void *)ptbr);
        ptbr = 0;
    }
    return 1;
}