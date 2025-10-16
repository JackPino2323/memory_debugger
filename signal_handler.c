#include "memdebug.h"
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static memory_map_t g_memory_map;
static int g_initialized = 0;

static void segfault_handler(int sig, siginfo_t *info, void *context __attribute__((unused))) {
    fault_context_t fault_ctx = {0};
    
    fault_ctx.fault_address = (uintptr_t)info->si_addr;
    fault_ctx.signal_number = sig;
    
    // Find the memory region containing the fault address
    fault_ctx.fault_region = (memory_region_t *)find_memory_region(&g_memory_map, fault_ctx.fault_address);
    
    // Capture backtrace
    fault_ctx.backtrace_size = backtrace(fault_ctx.backtrace, MAX_BACKTRACE_SIZE);
    fault_ctx.symbols = backtrace_symbols(fault_ctx.backtrace, fault_ctx.backtrace_size);
    
    // Print the fault report
    print_fault_report(&fault_ctx);
    
    // Clean up
    if (fault_ctx.symbols) {
        free(fault_ctx.symbols);
    }
    
    // Exit with the original signal
    _exit(sig);
}

void memdebug_init(void) {
    if (g_initialized) {
        return;
    }
    
    // Parse the memory map
    if (parse_memory_map(&g_memory_map) < 0) {
        fprintf(stderr, "memdebug: Failed to parse memory map\n");
        return;
    }
    
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_sigaction = segfault_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    
    if (sigaction(SIGSEGV, &sa, NULL) != 0) {
        perror("memdebug: Failed to install SIGSEGV handler");
        return;
    }
    
    if (sigaction(SIGBUS, &sa, NULL) != 0) {
        perror("memdebug: Failed to install SIGBUS handler");
        return;
    }
    
    g_initialized = 1;
}

void memdebug_cleanup(void) {
    g_initialized = 0;
}
