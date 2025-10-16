#include "memdebug.h"
#include <stdio.h>
#include <string.h>

const char *analyze_fault(const fault_context_t *context) {
    uintptr_t addr = context->fault_address;
    
    // Check for null pointer dereference
    if (addr < 0x1000) {
        return "NULL pointer dereference";
    }
    
    // Check if we have region information
    if (context->fault_region) {
        switch (context->fault_region->type) {
            case MEMORY_REGION_STACK:
                return "Stack access violation";
            case MEMORY_REGION_HEAP:
                return "Heap access violation";
            case MEMORY_REGION_CODE:
                if (strstr(context->fault_region->permissions, "r--")) {
                    return "Write to read-only code section";
                }
                return "Code section access violation";
            case MEMORY_REGION_DATA:
                return "Data section access violation";
            default:
                break;
        }
    }
    
    // Check for common patterns based on address ranges
    if (addr >= 0x7f0000000000ULL && addr < 0x800000000000ULL) {
        return "Likely stack overflow or stack corruption";
    }
    
    if (addr >= 0x100000000ULL && addr < 0x200000000ULL) {
        return "Likely heap access violation";
    }
    
    return "Access to unmapped memory";
}

void print_fault_report(const fault_context_t *context) {
    printf("========================================\n");
    printf("SEGMENTATION FAULT DETECTED\n");
    printf("========================================\n");
    printf("Signal: SIGSEGV (%d)\n", context->signal_number);
    printf("Fault Address: 0x%016lx\n", context->fault_address);
    printf("Diagnosis: %s\n", analyze_fault(context));
    printf("\n");
    
    // Provide explanation based on diagnosis
    const char *diagnosis = analyze_fault(context);
    if (strcmp(diagnosis, "NULL pointer dereference") == 0) {
        printf("The program attempted to access memory at address 0x0,\n");
        printf("which typically indicates dereferencing a NULL pointer.\n");
    } else if (strcmp(diagnosis, "Stack access violation") == 0) {
        printf("The program attempted to access memory in the stack region,\n");
        printf("possibly due to stack overflow or stack corruption.\n");
    } else if (strcmp(diagnosis, "Heap access violation") == 0) {
        printf("The program attempted to access memory in the heap region,\n");
        printf("possibly accessing freed memory or buffer overflow.\n");
    } else if (strstr(diagnosis, "read-only")) {
        printf("The program attempted to write to read-only memory.\n");
    } else {
        printf("The program attempted to access memory that is not mapped\n");
        printf("or accessible to the process.\n");
    }
    
    printf("\n");
    
    // Print stack trace if available
    if (context->backtrace_size > 0 && context->symbols) {
        printf("Stack Trace:\n");
        for (int i = 0; i < context->backtrace_size && i < 10; i++) {
            printf("  #%d  %s\n", i, context->symbols[i] ? context->symbols[i] : "???");
        }
        printf("\n");
    }
    
    // Print memory region information
    if (context->fault_region) {
        printf("Memory Region: ");
        switch (context->fault_region->type) {
            case MEMORY_REGION_STACK:
                printf("Stack");
                break;
            case MEMORY_REGION_HEAP:
                printf("Heap");
                break;
            case MEMORY_REGION_CODE:
                printf("Code");
                break;
            case MEMORY_REGION_DATA:
                printf("Data");
                break;
            default:
                printf("Unknown");
                break;
        }
        printf(" (0x%lx-0x%lx, %s)\n", 
               context->fault_region->start, 
               context->fault_region->end,
               context->fault_region->permissions);
    } else {
        printf("Memory Region: Unmapped (not allocated)\n");
    }
    
    printf("========================================\n");
}
