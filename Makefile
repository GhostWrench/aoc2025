# Basic makefile for compiling and running tests for a very basic project
SHELL = /bin/sh
.SUFFIXES:

# Build options
BUILD_CFG ?= debug

# Project configuration
PROJECT_NAME = aoc2025

# Directory definitions (defaults recommended by GNU)
srcdir = .

# Build Settings
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++23 -I$(srcdir)/include
ifeq ($(BUILD_CFG),debug)
CXXFLAGS += -g -O0 -DDEBUG
endif
LDFLAGS = 

# Find all the source and header files
EXEC_SRCS = $(wildcard $(srcdir)/src/*.cc)

# Calculate names of the build artifacts and outputs
EXECS = $(patsubst $(srcdir)/src/%.cc,./output/${BUILD_CFG}/%,$(EXEC_SRCS))

# Build targets
.PHONY: clean

all: $(EXECS)

./output/${BUILD_CFG}/% : $(srcdir)/src/%.cc
	@mkdir -p ./output/${BUILD_CFG}
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf ./output/*
