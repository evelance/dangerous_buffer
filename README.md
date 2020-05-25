# Dangerous buffer for testing purposes

## About

Allocates a buffer with a "dangerous zone" immediately afterwards for testing purposes.
Any access beyond the useable area by even one byte triggers a SIGSEGV.

It is intended to be small so that it can be easily copied into C++ unit test programs.

**License**: Public Domain

**Dependencies**: mmap

Just copy this into your test program:

```C
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/**
    Allocates a buffer with a "dangerous zone" immediately afterwards for testing purposes.
    Dispose with: munmap(bufmm, buflen);
     * len: Length of the useable buffer in bytes. The buffer is not aligned.
     * protect_last: PROT_NONE to disallow all acesses, PROT_READ to allow only read
     * mmptr: Pointer to the mmap mapping
     * mmlen: Length of the mmap mapping
     Returns: Pointer to the usable buffer (identical to mmptr when len % PAGE_SIZE == 0)
*/
static char* dangerous_buffer(size_t len, int protect_last, void*& mmptr, size_t& mmlen)
{
    #ifndef PAGE_SIZE
        #define PAGE_SIZE 4096
    #endif
    mmlen = (len &~ (PAGE_SIZE - 1)) + ((len % PAGE_SIZE) ? 2 : 1) * PAGE_SIZE; // One extra page
    mmptr = mmap(NULL, mmlen, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (mmptr == MAP_FAILED) {
        perror("mmap in dangerous_buffer");
        exit(-1);
    }
    if (mprotect((char*)mmptr + mmlen - PAGE_SIZE, PAGE_SIZE, protect_last)) { // Protect last page
        perror("mprotect in dangerous_buffer");
        exit(-1);
    }
    // Directly after the user pointer is the protected page
    return (char*)mmptr + ((len % PAGE_SIZE) ? (PAGE_SIZE - (len % PAGE_SIZE)) : 0);
}
```


