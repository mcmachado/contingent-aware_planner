# Makefile
#
# Author: Marlos C. Machado

CXX := g++

ALE := ../../src/ALE

# Set this to 1 to enable SDL and display_screen
USE_SDL := 1

# -O3 Optimize code (urns on all optimizations specified by -O2 and also turns on the -finline-functions, -funswitch-loops, -fpredictive-commoning, -fgcse-after-reload, -ftree-loop-vectorize, -ftree-slp-vectorize, -fvect-cost-model, -ftree-partial-pre and -fipa-cp-clone options).
# -D__USE_SDL Ensures we can use SDL to see the game screen
# -D_GNU_SOURCE=1 means the compiler will use the GNU standard of compilation, the superset of all other standards under GNU C libraries.
# -D_REENTRANT causes the compiler to use thread safe (i.e. re-entrant) versions of several functions in the C library.
FLAGS := -O3 -I$(ALE)/src -D_GNU_SOURCE=1 -D_REENTRANT -std=c++11 -Wno-deprecated-declarations -Wall -g -MMD

EXE := contingent-aware_planner.exe

# Search for library 'ale' and library 'z' when linking.
LDFLAGS := -L$(ALE) -lale -lz -lm

ifeq ($(strip $(USE_SDL)), 1)
  FLAGS +=  -D__USE_SDL `sdl-config --cflags`
  LDFLAGS += -lSDL -lSDL_gfx -lSDL_image `sdl-config --cflags --libs`
endif

all: $(EXE)

contingent-aware_planner.exe: bin/contingency.o
	$(CXX) $(FLAGS) bin/contingency.o $(LDFLAGS) -o $(EXE)

bin/contingency.o: contingency.cpp
	$(CXX) $(FLAGS) -c contingency.cpp -o bin/contingency.o

clean:
	rm -rf $(OBJS) $(EXE) bin/*

#This command needs to be executed in a osX before running the code:
#export DYLD_LIBRARY_PATH="${DYLD_LIBRARY_PATH}:/Users/machado/Research/src/ALE/"
