SHELL := /bin/bash


#default compiler settings
CC = g++
OPT = -O3 -g -std=c++0x
LDFLAGS = -lm

# set pattern conversion name
Gen_EXE = pat_gen
Gen_SRC = main.cc
Gen_OBJ = Test.o

# compilation for runs
all: track generate link

 track : CLCT.cc CLCT.h Set.cpp Set.h commands.h commands.cpp debug.h eth.h eth_lib.cpp eth_lib.h utils.cpp utils.h CSCConstants.h CSCConstants.cpp
	$(CC) $(OPT) -c CLCT.cc -o CLCT.o
	$(CC) $(OPT) -c Set.cpp -o Set.o
	$(CC) $(OPT) -c commands.cpp -o commands.o
	$(CC) $(OPT) -c eth_lib.cpp -o eth_lib.o
	$(CC) $(OPT) -c utils.cpp -o utils.o
	$(CC) $(OPT) -c CSCConstants.cpp -o consts.o

 generate : main.cc CLCT.cc CLCT.h Set.cpp Set.h CSCConstants.h
	$(CC) $(OPT) -c main.cc -o main.o
#	$(CC)  $(OPT) $(Gen_SRC) -o $(Gen_EXE) $(LDFLAGS)

 link : CLCT.o main.o Set.o commands.o eth_lib.o utils.o consts.o
	$(CC) main.o CLCT.o Set.o commands.o eth_lib.o utils.o consts.o -o $(Gen_EXE) -lm
	/bin/rm -rf *.o

clean:
	rm -rf *.o $(Gen_EXE)
