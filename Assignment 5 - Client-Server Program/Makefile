###
##  Makefile for Programming Assignment 5
###
CC = gcc
CFLAGS = -O -g -Wall -Wextra
OBJFILES = server.o helpers.o client.o
TARGETS = server client
.PHONY: all clean client server

#  Build all targets
all:
	$(CC) $(CFLAGS) -o server helpers.c server.c
	$(CC) $(CFLAGS) -o client helpers.c client.c

#  Run the server program
server:
	@./server -t udp -p 3015

#  Run the client program
client:
	@./client -s 127.0.1.1 -t udp -p 3015 -x 100

#  Cleanup object files and logs
clean: 
	rm -f $(OBJFILES) $(TARGETS) *.txt *.log *~
