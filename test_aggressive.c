#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Testing aggressive buffer overflow...\n");
    
    int buffer[10];
    printf("Buffer allocated at %p\n", (void*)buffer);
    
    printf("About to write way beyond buffer bounds...\n");
    
    // Write to a very far address that's definitely unmapped
    int *far_ptr = (int*)0x1000000000000ULL; // Very high address
    *far_ptr = 42;
    
    printf("This line should never be reached\n");
    return 0;
}
