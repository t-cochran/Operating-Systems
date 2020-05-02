####################################################
#             Programming assignment 2             #
#                  Thomas Cochran                  #
#                    CSPB 3753                     #
####################################################

********
 Files:
********

shell.c
    The main shell package containing the read-evaluate loop, parser, 
    forking and execing.

helpers.{c, h}
    Helper routines used by the shell, such as checking file directories, 
    displaying the prompt, re-allocing the command line buffer, etc.

job_list.{c, h}
    Data structure to track jobs running from the shell. Background
    processes can be tracked at any time by typing "jobs" in the shell. 

Makefile
    Builds the shell.

big.txt
    A ~6 megabyte .txt file used to stress test 'cat' with pipe commands.


**************************
 Build and run the shell
**************************
To build the shell, type "make" in a bash terminal in the shell directory.

To run the shell, type one of the following in a bash terminal:

    (1) "./shell" : This will run the shell with 'bash' as a parent process.

    (2) "make shell" : This will run the shell with 'make' as a parent process.

    (3) "make memcheck" : This will run the shell with valgrind to check for memory leaks.

To cleanup object files before rebuilding, type "make clean" in a bash terminal. 


*******************************
 Current features of the shell
*******************************
As per the assignment guidelines the shell should be able to:

    (1) 'cd ..' and 'cd /dir/' to change directories

    (2) Most /bin/ commands, prepended with /bin/ or not. E.g. 'ls -l'

    (3) Single pipe functionality. E.g. 'echo test | grep t'

    (4) Foreground and background support. E.g. 'sleep 10s &'
    
    (5) Built-in commands:
            > 'tokenize <string>'     Prints a string's tokens
            > 'pwd'                   Print the current working directory
            > 'quit' or 'DONE'        Exit the shell                   

I attempted to implement multiple pipes and couldn't get it to work. 
I left the remnants of my approach in the code and can explain it if 
asked.
