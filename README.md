# Memory Debugger

A simple memory debugger for C programs that catches segmentation faults and provides human-readable explanations of what went wrong.

## Features

- **Signal Handler Based**: Uses `sigaction()` to intercept SIGSEGV and SIGBUS signals
- **Memory Map Analysis**: Parses `/proc/self/maps` to understand memory layout
- **Fault Classification**: Identifies common segfault patterns like:
  - NULL pointer dereference
  - Stack overflow
  - Heap access violation
  - Write to read-only memory
  - Access to unmapped memory
- **Stack Trace**: Shows call stack when available
- **Works with Any Binary**: Uses `LD_PRELOAD` to attach to existing programs

## Building

```bash
make
```

This creates `libmemdebug.so` in the current directory.

## Usage

### Method 1: Using the wrapper script (Recommended)

```bash
./memdebug /path/to/program [args...]
```

The wrapper script automatically detects the operating system and uses the appropriate preload mechanism:
- **macOS**: Uses `DYLD_INSERT_LIBRARIES`
- **Linux**: Uses `LD_PRELOAD`

### Method 2: Direct preload

**On macOS:**
```bash
DYLD_INSERT_LIBRARIES=./libmemdebug.so /path/to/program [args...]
```

**On Linux:**
```bash
LD_PRELOAD=./libmemdebug.so /path/to/program [args...]
```

## Example Output

```
========================================
SEGMENTATION FAULT DETECTED
========================================
Signal: SIGSEGV (11)
Fault Address: 0x0000000000000000
Diagnosis: NULL pointer dereference

The program attempted to access memory at address 0x0,
which typically indicates dereferencing a NULL pointer.

Stack Trace:
  #0  main at test_null+0x1234
  #1  __libc_start_main at libc.so.6+0x5678

Memory Region: Unmapped (not allocated)
========================================
```

## Testing

Compile and run the example programs:

```bash
# Build the debugger
make

# Compile test programs
gcc -o examples/test_null examples/test_null.c
gcc -o examples/test_overflow examples/test_overflow.c
gcc -o examples/test_use_after_free examples/test_use_after_free.c
gcc -o examples/test_aggressive examples/test_aggressive.c

# Run with debugger
./memdebug examples/test_null
./memdebug examples/test_aggressive
```

## How It Works

1. **Initialization**: When loaded via preload mechanism, the constructor function runs and:
   - Parses memory map using OS-specific APIs (Mach on macOS, `/proc/self/maps` on Linux)
   - Installs signal handlers for SIGSEGV and SIGBUS

2. **Fault Detection**: When a segfault occurs:
   - The signal handler extracts the fault address from `siginfo_t`
   - Captures a stack trace using `backtrace()`
   - Finds the memory region containing the fault address
   - Analyzes the fault type based on address and region

3. **Analysis**: The analyzer classifies faults by:
   - Checking if address is near NULL (null pointer)
   - Examining the memory region type (stack, heap, code, data)
   - Looking for common address patterns

4. **Reporting**: Generates a human-readable report with:
   - Signal type and fault address
   - Diagnosis of the likely cause
   - Explanation of what the fault means
   - Stack trace (if available)
   - Memory region information

## Limitations

- **Cross-platform**: Works on macOS and Linux, but uses different APIs:
  - macOS: Uses Mach APIs (`vm_region_64`) for memory mapping
  - Linux: Uses `/proc/self/maps` for memory mapping
- Requires programs to be compiled with debug symbols for meaningful stack traces
- Basic analysis - doesn't detect all possible fault patterns
- Signal handler approach means it can't prevent the crash, only explain it
- Some memory access patterns may not trigger segfaults due to OS-specific protections

## Architecture

The debugger is split into focused modules:

- `memmap.c`: Parses memory maps and identifies regions
- `analyzer.c`: Classifies fault types and generates explanations  
- `signal_handler.c`: Handles signals and collects fault information
- `memdebug.c`: Main entry point with constructor/destructor

This modular design makes the code readable and maintainable.
