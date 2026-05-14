#define _GNU_SOURCE
#include "util.h"
#include <stdio.h>      // for printf
#include <stdlib.h>     // for atoi (and malloc() which you'll likely use)
#include <sys/mman.h>   // for mmap() which you'll likely use
#include <stdalign.h>
#include <string.h>

alignas(4096) volatile char global_array[4096 * 32];

void labStuff(int which) {
    if (which == 0) {
        /* do nothing */
    } else if (which == 1) {
        
        memset((void*)global_array, 0, 4096); // writes in just 1 page => 1 page fault
        
        int flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED;
        (void*)mmap(
            (void*)global_array,           // addr
            4096,                   // size
            PROT_READ | PROT_WRITE, // prot 
            flags,                  // flags
            -1,                     // fd (none)
            0                       // offset
        ); // remap first 4096 bytes (1 page) to a fresh AoD page
        
        memset((void*)global_array, 0, 8192); // write crosses 2 pages => 2 page fault
        
        memset((void*)global_array, 1, 8192); // should incur no page fault

    } else if (which == 2) {
        long one_mib = 1024*1024; // (guarenteed to be aligned bc a multiple of pg size)
        int flags = MAP_PRIVATE | MAP_ANON;
        char* mapped_addr = mmap(
            0,                      // addr (automatic)
            one_mib,                // size
            PROT_READ | PROT_WRITE, // prot 
            flags,                  // flags
            -1,                     // fd (none)
            0                       // offset
        ); // map 1MiB (256 pages) to fresh AoD pages
        
        // NOTE: doesn't fault on just accesses bc of compiler optimizations (?)
        mapped_addr[0] = '1';    // fault
        mapped_addr[2] = '1';    // no fault
        mapped_addr[4097] = '1'; // fault
        mapped_addr[6767] = '1'; // no fault
        
    } else if (which == 3) {
        long one_mib = 1024*1024; // (guarenteed to be aligned bc a multiple of pg size)
        int flags = MAP_PRIVATE | MAP_ANON;
        char* mapped_addr = mmap(
            0,                      // addr (automatic)
            one_mib,                // size
            PROT_READ | PROT_WRITE, // prot 
            flags,                  // flags
            -1,                     // fd (none)
            0                       // offset
        ); // map 1MiB (256 pages) to fresh AoD pages
        
        // touch 128KiB (32 pages) of the 256
        // fewest page faults = 1 per page
        for (int i = 0; i < 32; i++) {
            mapped_addr[4096*i] = '1';
        }

    } else if (which == 4) {
        // heap location: 55555555a000-5555555bbfff
        int flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED_NOREPLACE;
        long heap_end = 0x5555555bbfff;
        long offset = 0x200000;
        long addr = heap_end + offset;
        if (addr % 4096 != 0) addr += (4096 - addr%4096);
        char* mapped_addr = mmap(
            (void*)addr,            // addr
            4096,                   // size
            PROT_READ | PROT_WRITE, // prot 
            flags,                  // flags
            -1,                     // fd (none)
            0                       // offset
        ); // map 1KiB (1 page) to a fresh AoD page
         
        mapped_addr[0] = '1'; // touch to trigger AoD
    } else if (which == 5) {
        // heap location: 55555555a000-5555555bbfff
        int flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED_NOREPLACE;
        long heap_end = 0x5555555bbfff;
        long offset = 0x10000000000;
        long addr = heap_end + offset;
        if (addr % 4096 != 0) addr += (4096 - addr%4096);
        char* mapped_addr = mmap(
            (void*)addr,            // addr
            4096,                   // size
            PROT_READ | PROT_WRITE, // prot 
            flags,                  // flags
            -1,                     // fd (none)
            0                       // offset
        ); // map 1KiB (1 page) to a fresh AoD page
         
        mapped_addr[0] = '1'; // touch to trigger AoD
    }

}

int main(int argc, char **argv) {
    int which = 0;
    if (argc > 1) {
        which = atoi(argv[1]);
    } else {
        fprintf(stderr, "Usage: %s NUMBER\n", argv[0]);
        return 1;
    }
    printf("Memory layout:\n");
    print_maps(stdout);
    printf("\n");
    printf("Initial state:\n");
    force_load();
    struct memory_record r1, r2;
    record_memory_record(&r1);
    print_memory_record(stdout, NULL, &r1);
    printf("---\n");

    printf("Running labStuff(%d)...\n", which);

    labStuff(which);

    printf("---\n");
    printf("Afterwards:\n");
    record_memory_record(&r2);
    print_memory_record(stdout, &r1, &r2);
    print_maps(stdout);
    return 0;
}
