#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

static void segfault_handler(int sig, siginfo_t *info, void *context __attribute__((unused))) {
    printf("========================================\n");
    printf("SEGMENTATION FAULT DETECTED\n");
    printf("========================================\n");
    printf("Signal: SIGSEGV (%d)\n", sig);
    printf("Fault Address: %p\n", info->si_addr);
    
    // Basic analysis
    if (info->si_addr == NULL) {
        printf("Diagnosis: NULL pointer dereference\n");
        printf("\nThe program attempted to access memory at address 0x0,\n");
        printf("which typically indicates dereferencing a NULL pointer.\n");
    } else {
        printf("Diagnosis: Invalid memory access\n");
        printf("\nThe program attempted to access memory that is not mapped\n");
        printf("or accessible to the process.\n");
    }
    
    // Stack trace
    void *buffer[32];
    int nptrs = backtrace(buffer, 32);
    char **symbols = backtrace_symbols(buffer, nptrs);
    
    if (symbols != NULL) {
        printf("\nStack Trace:\n");
        for (int i = 0; i < nptrs && i < 10; i++) {
            printf("  #%d  %s\n", i, symbols[i]);
        }
        free(symbols);
    }
    
    printf("========================================\n");
    _exit(sig);
}

__attribute__((constructor))
static void init_debugger(void) {
    printf("memdebug: Initializing simple debugger...\n");
    
    struct sigaction sa;
    sa.sa_sigaction = segfault_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    
    if (sigaction(SIGSEGV, &sa, NULL) != 0) {
        perror("memdebug: Failed to install SIGSEGV handler");
        return;
    }
    
    printf("memdebug: Signal handler installed successfully\n");
}
