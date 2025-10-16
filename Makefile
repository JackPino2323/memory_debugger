CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC -g -O2
LDFLAGS = -shared -ldl

# macOS specific libraries
ifeq ($(shell uname),Darwin)
    LDFLAGS += -framework CoreFoundation
endif

SRCDIR = src
INCDIR = include
OBJDIR = obj
LIBDIR = .

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(LIBDIR)/libmemdebug.so

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
	@echo "To use memdebug, run:"
	@echo "  LD_PRELOAD=./libmemdebug.so your_program"
	@echo "  or use the memdebug wrapper script"
