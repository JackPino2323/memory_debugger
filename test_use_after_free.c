#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Testing use-after-free...\n");
    
    int *ptr = malloc(sizeof(int));
    if (!ptr) {
        printf("malloc failed\n");
        return 1;
    }
    
    *ptr = 42;
    printf("Allocated memory at %p, value = %d\n", (void*)ptr, *ptr);
    
    free(ptr);
    printf("Memory freed\n");
    
    printf("About to access freed memory...\n");
    
    // This will cause a segfault
    *ptr = 100;
    
    printf("This line should never be reached\n");
    return 0;
}
