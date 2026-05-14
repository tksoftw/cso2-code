#include <stddef.h>
#include <string.h>
#include "tlb.h"

#define NUM_SETS 16
#define NUM_WAYS 4

typedef struct {
    int valid;
    size_t vpn;
    size_t ppn;
    unsigned long last_access;
} tlb_entry;

static tlb_entry tlb[NUM_SETS][NUM_WAYS];
static unsigned long access_counter = 0;

static inline size_t get_vpn(size_t va) { return va >> POBITS; } // high bits don't matter
static inline size_t get_offset(size_t va) { return va & ((1UL << POBITS) - 1); }
static inline size_t get_set(size_t vpn) { return vpn % NUM_SETS; }
static inline tlb_entry *tlb_at(size_t set, size_t way) { return &tlb[set][way]; }

void tlb_clear() {
    // invalidate all entries and reset the access counter
    access_counter = 0;
    memset(tlb, 0, sizeof(tlb));
}

int tlb_peek(size_t va) {
    size_t vpn = get_vpn(va);
    size_t set = get_set(vpn);

    // look for the entry in the TLB
    int found = -1;
    for (int i = 0; i < NUM_WAYS; ++i) {
        tlb_entry *e = tlb_at(set, i);
        if (e->valid && e->vpn == vpn) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        return 0; // not in the TLB
    }

    // calculate the lru status of the entry of the valid entries in the set
    tlb_entry *f = tlb_at(set, found);
    int lru_status = 1;
    for (int i = 0; i < NUM_WAYS; ++i) {
        tlb_entry *e = tlb_at(set, i);
        if (i != found && e->valid &&
            e->last_access > f->last_access) {
            ++lru_status;
        }
    }
    return lru_status;
}

size_t tlb_translate(size_t va) {
    size_t vpn = get_vpn(va);
    size_t offset = get_offset(va);
    size_t set = get_set(vpn);

    ++access_counter;

    // look for the entry in the TLB
    for (int i = 0; i < NUM_WAYS; ++i) {
        tlb_entry *e = tlb_at(set, i);
        if (e->valid && e->vpn == vpn) {
            e->last_access = access_counter;
            return (e->ppn << POBITS) | offset; // return the physical address
        }
    }

    // cache miss, translate the va to ppn and update the TLB
    size_t pa = translate(vpn << POBITS);
    if (pa == (size_t)-1) {
        return (size_t)-1;
    }

    size_t ppn = pa >> POBITS;

    // find the lru entry in the set to evict
    size_t lru = (size_t)-1;
    for (size_t i = 0; i < NUM_WAYS; ++i) {
        if (!tlb_at(set, i)->valid) {
            lru = i;
            break;
        }
    }
    if (lru == (size_t)-1) {
        lru = 0;
        for (size_t i = 1; i < NUM_WAYS; ++i) {
            if (tlb_at(set, i)->last_access < tlb_at(set, lru)->last_access) {
                lru = i; // found an entry that was accessed less recently
            }
        }
    }

    // update the TLB entry using designated initializer notation
    *tlb_at(set, lru) = (tlb_entry){
        .valid = 1,
        .vpn = vpn,
        .ppn = ppn,
        .last_access = access_counter,
    };

    return (ppn << POBITS) | offset; // return the physical address
}
