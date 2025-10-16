#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Testing NULL pointer dereference...\n");
    
    int *ptr = NULL;
    printf("About to dereference NULL pointer...\n");
    
    // This will cause a segfault
    *ptr = 42;
    
    printf("This line should never be reached\n");
    return 0;
}
