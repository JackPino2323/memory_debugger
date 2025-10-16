#include "memdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <mach/vm_region.h>
#endif

int parse_memory_map(memory_map_t *map) {
#ifdef __APPLE__
    // macOS implementation using Mach APIs
    map->region_count = 0;
    
    vm_map_t target_task = mach_task_self();
    vm_address_t address = 0;
    vm_size_t size = 0;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object_name;
    
    while (vm_region_64(target_task, &address, &size, VM_REGION_BASIC_INFO_64,
                       (vm_region_info_t)&info, &info_count, &object_name) == KERN_SUCCESS) {
        if (map->region_count >= MAX_MEMORY_REGIONS) break;
        
        memory_region_t *region = &map->regions[map->region_count];
        region->start = address;
        region->end = address + size;
        
        // Convert protection flags to permission string
        char perms[8] = "---";
        if (info.protection & VM_PROT_READ) perms[0] = 'r';
        if (info.protection & VM_PROT_WRITE) perms[1] = 'w';
        if (info.protection & VM_PROT_EXECUTE) perms[2] = 'x';
        strncpy(region->permissions, perms, sizeof(region->permissions) - 1);
        
        // Classify region type based on protection and address
        region->type = classify_region_type("", perms);
        strcpy(region->pathname, ""); // No pathname available from Mach API
        
        map->region_count++;
        address += size;
    }
    
    return map->region_count;
#else
    // Linux implementation using /proc/self/maps
    FILE *maps_file = fopen("/proc/self/maps", "r");
    if (!maps_file) {
        return -1;
    }

    map->region_count = 0;
    char line[512];
    
    while (fgets(line, sizeof(line), maps_file) && map->region_count < MAX_MEMORY_REGIONS) {
        memory_region_t *region = &map->regions[map->region_count];
        
        // Parse: start-end permissions offset dev inode pathname
        unsigned long start, end, offset, dev_major, dev_minor, inode;
        char permissions[8];
        char pathname[256] = {0};
        
        int parsed = sscanf(line, "%lx-%lx %7s %lx %lx:%lx %lu %255s",
                           &start, &end, permissions, &offset,
                           &dev_major, &dev_minor, &inode, pathname);
        
        if (parsed >= 3) {
            region->start = (uintptr_t)start;
            region->end = (uintptr_t)end;
            strncpy(region->permissions, permissions, sizeof(region->permissions) - 1);
            strncpy(region->pathname, pathname, sizeof(region->pathname) - 1);
            region->type = classify_region_type(pathname, permissions);
            map->region_count++;
        }
    }
    
    fclose(maps_file);
    return map->region_count;
#endif
}

const memory_region_t *find_memory_region(const memory_map_t *map, uintptr_t address) {
    for (int i = 0; i < map->region_count; i++) {
        const memory_region_t *region = &map->regions[i];
        if (address >= region->start && address < region->end) {
            return region;
        }
    }
    return NULL;
}

memory_region_type_t classify_region_type(const char *pathname, const char *permissions) {
    if (strlen(pathname) == 0) {
        // Anonymous mapping
        if (strstr(permissions, "rw")) {
            return MEMORY_REGION_HEAP;
        }
        return MEMORY_REGION_UNKNOWN;
    }
    
    if (strstr(pathname, "[stack]")) {
        return MEMORY_REGION_STACK;
    }
    
    if (strstr(pathname, "[heap]")) {
        return MEMORY_REGION_HEAP;
    }
    
    if (strstr(pathname, ".so") || strstr(pathname, ".dylib")) {
        return MEMORY_REGION_CODE;
    }
    
    if (strstr(permissions, "r-x")) {
        return MEMORY_REGION_CODE;
    }
    
    if (strstr(permissions, "rw-")) {
        return MEMORY_REGION_DATA;
    }
    
    return MEMORY_REGION_UNKNOWN;
}
