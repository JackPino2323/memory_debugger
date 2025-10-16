#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Testing buffer overflow...\n");
    
    int buffer[10];
    printf("Buffer allocated at %p\n", (void*)buffer);
    
    printf("About to write beyond buffer bounds...\n");
    
    // This will cause a segfault by writing beyond the buffer
    buffer[1000] = 42;
    
    printf("This line should never be reached\n");
    return 0;
}
