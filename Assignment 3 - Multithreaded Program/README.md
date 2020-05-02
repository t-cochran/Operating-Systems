####################################################
#             Programming assignment 3             #
#                  Thomas Cochran                  #
#                    CSPB 3753                     #
####################################################

********
 Files:
********

multi-lookup.c
    The main program containing parser and converter thread routines.

helpers.{c, h}
    Helper functions used by the main program, such as initializing 
    data structures, adding log entries, or reading lines from input files.

DS_stack.{c, h}
    The stack data structure used as the shared buffer for this assignment.

util.{c, h}
    Resolves domain names to IP addresses. Provided by the assignment 
    (thanks Dr. Knox). Slightly modified by me to collect multiple 
    IP addresses.

wrappers.{c, h}
    Error handling wrappers for a variety of pthread library functions.

Makefile
    Builds the main program.

logs/*.log
    Folder to store log files when running the main program. 

input/*.txt
    Folder containing input files with various domain names.

headers/*.h
    Folder containing all header files.


****************************
 Build and run main program
****************************
To build the program, type "make" in a bash terminal in the multi-lookup.c file directory.

To run the program, the following command-line argument format must be passed:

  ./multi-lookup <# parsing threads> <#conversion threads> <parsing log> <converter log> <datafiles>
  
  Example:

      ./multi-lookup 10 10 logs/parser.log logs/results.log input/names1.txt
  
The above list of command line arguments will run the main program with 10 parser threads, 10 converter threads, store logs to log/parser.log and logs/convert.log, and use the file input/names.txt as the input file for domain names.

******************
 Makefile options
******************
The makefile has some automated options to run and test the program:

    (1) "make main" 
    Runs the main program with a default 20 parser, 20  converter threads over all 15 documents within the input folder and stores output to the log folder.

    (2) "make test" 
    Runs the main program with 10 parser, 10 converter threads over 1 document and repeat this N times to check for race conditions and/or deadlock.

    (3) "make big"
    Runs the main program with 100 parser, 100 converter threads over 1 document
    containing 5000 domain names.

    (4) "make messy"
    Runs the main program with 20 parser, 20 converter threads over 1 document
    containing 23 domain names hidden in lines full of junk characters.

    (5) "make gdb"
    Runs the main program with settings from (2) in GDB for debugging.

    (6) "make memcheck"
    Runs the main program with settings from (2) with valgrind to check for memory leaks.

To cleanup object files before rebuilding, type "make clean" in a bash terminal. 


******************
 Current features
******************
As per the assignment guidelines the program should be able to: 

    (1) Read and process command-line arguments.

    (2) Create parsing threads to read lines from a file.

    (3) Create conversion threads to take domain name and find IP address.

    (4) Communicate between threads using a protected shared buffer (e.g. a stack).
    
    (5) Use multiple parsing and converting threads which the user can specify.

    (6) Record progress of parser and converter threads to protected log files.

    (7) Handle more files than the number of parsing threads.


***********************
 Extra credit attempts
***********************
    (1) Extra Credit: Create an API for accessing input data (i.e. the readline function)

    (2) Extra Credit: Supports listing an arbitrary number of IP addresses.
    
