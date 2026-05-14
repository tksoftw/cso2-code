#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "config.h"

/* stub translate: matches the assignment's example stub */
size_t ptbr;
size_t translate(size_t va) {
    if (va < 0x1234000)
        return va + 0x20000;
    else if (va > 0x2000000 && va < 0x2345000)
        return va + 0x100000;
    else
        return -1;
}

#include "tlb.h"

int main() {
    printf("=== Example 1 ===\n");
    tlb_clear();
    assert(tlb_peek(0) == 0);
    assert(tlb_translate(0) == 0x0020000);
    assert(tlb_peek(0) == 1);
    assert(tlb_translate(0x200) == 0x20200);
    assert(tlb_translate(0x400) == 0x20400);
    assert(tlb_peek(0) == 1);
    assert(tlb_peek(0x200) == 1);
    assert(tlb_translate(0x2001200) == 0x2101200);
    assert(tlb_translate(0x0005200) == 0x0025200);
    assert(tlb_translate(0x0008200) == 0x0028200);
    assert(tlb_translate(0x0002200) == 0x0022200);
    assert(tlb_peek(0x2001000) == 1);
    assert(tlb_peek(0x0001000) == 0);
    assert(tlb_peek(0x0004000) == 0);
    assert(tlb_peek(0x0005000) == 1);
    assert(tlb_peek(0x0008000) == 1);
    assert(tlb_peek(0x0002000) == 1);
    tlb_clear();
    assert(tlb_peek(0x2001000) == 0);
    assert(tlb_peek(0x0005000) == 0);
    assert(tlb_peek(0x0008000) == 0);
    assert(tlb_peek(0x0002000) == 0);
    assert(tlb_peek(0x0000000) == 0);
    assert(tlb_translate(0) == 0x20000);
    assert(tlb_peek(0) == 1);
    printf("Example 1 passed!\n");

    printf("=== Example 2 ===\n");
    tlb_clear();
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0801200) == 0x0821200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    assert(tlb_translate(0x0501200) == 0x0521200);
    assert(tlb_translate(0x0A01200) == 0x0A21200);
    assert(tlb_peek(0x0001200) == 0);
    assert(tlb_peek(0x2101200) == 0);
    assert(tlb_peek(0x2301200) == 3);
    assert(tlb_peek(0x0501200) == 2);
    assert(tlb_peek(0x0801200) == 4);
    assert(tlb_peek(0x0A01000) == 1);
    assert(tlb_translate(0x2301800) == 0x2401800);
    assert(tlb_peek(0x0001000) == 0);
    assert(tlb_peek(0x2101000) == 0);
    assert(tlb_peek(0x2301000) == 1);
    assert(tlb_peek(0x0501000) == 3);
    assert(tlb_peek(0x0801000) == 4);
    assert(tlb_peek(0x0A01000) == 2);
    for (int i = 0; i < 16; i += 1) {
        if (i != 1) {
            assert(tlb_translate(0x400000 + (i << 12)) == (0x420000 + (i << 12)));
            assert(tlb_translate(0x500000 + (i << 12)) == (0x520000 + (i << 12)));
            assert(tlb_translate(0x600000 + (i << 12)) == (0x620000 + (i << 12)));
            assert(tlb_translate(0x700000 + (i << 12)) == (0x720000 + (i << 12)));
            assert(tlb_translate(0x800000 + (i << 12)) == (0x820000 + (i << 12)));
            assert(tlb_peek(0x800000 + (i << 12)) == 1);
            assert(tlb_peek(0x400000 + (i << 12)) == 0);
            assert(tlb_peek(0x0001000) == 0);
            assert(tlb_peek(0x2101000) == 0);
            assert(tlb_peek(0x2301000) == 1);
            assert(tlb_peek(0x0501000) == 3);
            assert(tlb_peek(0x0801000) == 4);
            assert(tlb_peek(0x0A01000) == 2);
        }
    }
    tlb_clear();
    assert(tlb_peek(0x301000) == 0);
    assert(tlb_peek(0x501000) == 0);
    assert(tlb_peek(0x801000) == 0);
    assert(tlb_peek(0xA01000) == 0);
    assert(tlb_translate(0xA01200) == 0xA21200);
    printf("Example 2 passed!\n");

    printf("=== Example 3 ===\n");
    tlb_clear();
    assert(tlb_translate(0xA0001200) == -1);
    assert(tlb_peek(0xA0001000) == 0);
    assert(tlb_translate(0x1200) == 0x21200);
    assert(tlb_peek(0xA0001200) == 0);
    assert(tlb_peek(0x1000) == 1);
    assert(tlb_translate(0xA0001200) == -1);
    assert(tlb_translate(0xB0001200) == -1);
    assert(tlb_translate(0xC0001200) == -1);
    assert(tlb_translate(0xD0001200) == -1);
    assert(tlb_translate(0xE0001200) == -1);
    assert(tlb_peek(0x1000) == 1);
    assert(tlb_translate(0x1200) == 0x21200);
    printf("Example 3 passed!\n");

    printf("=== Example 4 ===\n");
    tlb_clear();
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0801200) == 0x0821200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    tlb_clear();
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x0001200) == 0x0021200);
    assert(tlb_translate(0x2101200) == 0x2201200);
    assert(tlb_translate(0x2301200) == 0x2401200);
    assert(tlb_translate(0x0011200) == 0x0031200);
    assert(tlb_peek(0x0001200) == 4);
    assert(tlb_peek(0x2101200) == 3);
    assert(tlb_peek(0x2301200) == 2);
    assert(tlb_peek(0x0011200) == 1);
    printf("Example 4 passed!\n");

    printf("\nAll examples passed!\n");
    return 0;
}
