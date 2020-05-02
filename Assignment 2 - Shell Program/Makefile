CC = gcc
CFLAGS = -Wall -O -g
TARGETS = shell.c helpers.c job_list.c
FILES = ./shell ./helpers ./job_list
.PHONY: shell memcheck clean

#  Compile shell target
all: $(TARGETS)
	$(CC) $(CFLAGS) -o shell job_list.c helpers.c shell.c

#  Run shell executable normally
shell: 
	@./shell

#  Run shell executable with valgrind
memcheck:
	@valgrind --leak-check=full --show-leak-kinds=all ./shell

#  Cleanup object files
clean: 
	rm -f $(FILES) *.o *~
