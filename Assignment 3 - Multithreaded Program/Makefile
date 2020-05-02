###
##  Makefile for Programming Assignment 3
###
CC = gcc
CFLAGS = -Wall -Wextra -O -g -pthread
OBJFILES = DS_stack.o util.o helpers.o wrappers.o multi-lookup.o
FILES = DS_stack.c util.c helpers.c wrappers.c multi-lookup.c
HDRS = helpers.h wrappers.h DS_stack.h util.h
TARGETS = multi-lookup
.PHONY: all main gdb memcheck clean messy test big

#  Input files containing domain names
INPUT_FILES = input/names1.txt input/names2.txt input/names3.txt input/names4.txt \
input/names5.txt input/names6.txt input/names7.txt input/names8.txt \
input/names9.txt input/names10.txt input/names11.txt input/names12.txt \
input/names13.txt input/names14.txt input/names15.txt

#  Build all targets
all: $(TARGETS)

#  Target program dependent on all linked object files
$(TARGETS): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGETS) $(FILES)

#  Run the main program
main:
	@./multi-lookup 20 20 logs/parser.log logs/results.log $(INPUT_FILES)

#  Run the program {1..N} times to check for race conditions or deadlock (thanks dr. knox!)
test:
	@for i in {1..1000}; do echo $i restart >> out.txt; ./multi-lookup 20 20 logs/parser.log logs/results.log input/names1.txt >> out.txt; done

#  Run the program with 100 parser, 100 converter threads and a single input file of 3000 domain names
big:
	@./multi-lookup 100 100 logs/parser.log logs/results.log input/big.txt

#  Run the program with 20 parsers, 20 converters on a single input file filled with junk characters
messy:
	@./multi-lookup 20 20 logs/parser.log logs/results.log input/messy.txt

#  Run the main program from GDB
gdb:
	@gdb --args ./multi-lookup 1 1 logs/parser.log logs/results.log input/names1.txt

#  Run the main program from valgrind
memcheck:
	@valgrind --leak-check=full --show-leak-kinds=all ./multi-lookup 10 10 logs/parser.log logs/results.log $(INPUT_FILES)

#  Cleanup object files and logs
clean: 
	rm -f $(OBJFILES) $(TARGETS) *.txt *.log *~