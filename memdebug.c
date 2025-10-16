#include "memdebug.h"
#include <stdio.h>

// Constructor function that runs when the library is loaded
__attribute__((constructor))
static void memdebug_constructor(void) {
    memdebug_init();
}

// Destructor function that runs when the library is unloaded
__attribute__((destructor))
static void memdebug_destructor(void) {
    memdebug_cleanup();
}
