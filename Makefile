# Name of the output program
NAME = RISK

# Board model
PLATFORM = TI84PCE

# Whether math routines are located in RAM (1 for yes, 0 for no)
COMPRESSED = 1

# Output format (8XP, 8X)
OUTFORMAT = 8XP

# Directory to build in
BINDIR = bin

# C compiler flags
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# Libraries to include
LIBS = -lgraphx -lfileioc -lkeypadc

# Include directory paths
INCLUDE =

# Source directory
SRC = src

# Automatically include all .c files in the source directory
CSOURCES := $(wildcard $(SRC)/*.c)
# No C++ sources in this project
CPPSOURCES :=

# Include the common build script
include $(shell cedev-config --makefile)
