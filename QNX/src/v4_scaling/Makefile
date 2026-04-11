# QNX ADAS V4 SCALING MAKEFILE (Robust Version)
# ---------------------------------------------------------
# This version uses VPATH to help 'make' find source files
# in subdirectories, even if run from a build folder.
# ---------------------------------------------------------

CC = qcc
PLATFORM ?= x86_64
CFLAGS = -Vgcc_nto$(PLATFORM) -Wall -O2 -I./common -I.

# Source directories
DIRS = system core sensors decision common
VPATH = $(DIRS)

# Source files (just the filenames, VPATH handles the rest)
SOURCES = orchestrator_8core.c \
          zonal_aggregator.c \
          fusion_engine.c \
          fleet_generator.c \
          decision_logic.c

# Object files
OBJS = $(SOURCES:.c=.o)

# Final Binary Name
TARGET = adas_v4_system

# Default Rule
all: $(TARGET)

# Linking Rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Compiling Rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup Rule
clean:
	rm -f *.o $(TARGET)

# Debug Rule to check paths
check:
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJS)"
	@echo "Current Dir: $$(pwd)"
	@echo "VPATH: $(VPATH)"
