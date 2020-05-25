#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <signal.h>

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


// Check if SIGSEGV was triggered
sig_atomic_t got_segv;

static void sigsegv_handler(int sig, siginfo_t* si, void* unused) {
    (void)sig; (void)unused;
    got_segv = 1;
    void* page_base = (void*)((uintptr_t)(si->si_addr) &~ (PAGE_SIZE - 1));
    if (mprotect(page_base, PAGE_SIZE, PROT_READ|PROT_WRITE)) {
        perror("mprotect in sigsegv_handler");
        exit(-1);
    }
}

int main() {
    // Setup signal handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(-1);
    }
    // Allocate various buffer sizes, write into the allowed area, and do one illegal write
    for (size_t len = 0; len < 3 * PAGE_SIZE; ++len) {
        void* bufmm = NULL;
        size_t buflen = 0;
        char* buf = dangerous_buffer(len, PROT_NONE, bufmm, buflen);
        got_segv = 0;
        for (size_t i = 0; i < len; ++i) {
            buf[i] = i;
        }
        assert(got_segv == 0);
        buf[len] = len;
        assert(got_segv == 1);
        munmap(bufmm, buflen);
    }
    return 0;
}

