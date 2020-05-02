####################################################
#             Programming assignment 4             #
#                  Thomas Cochran                  #
#                    CSPB 3753                     #
####################################################

********
 Files:
********
pager-lru.c
    Least recently used (LRU) paging algorithm.

pager-predict.{c, h}
    Predictive paging algorithm using markov chains.

PA4_README {folder}
    PAGER_PREDICT_INFO.pdf
        Outlines how I collected data to create the probability matrices used in 
        the predictive pager solution. Also includes all the raw data that led me
        to the final matrices that are included in pager-predict.h.
    README.md
        
Makefile
    Builds pager-lru and pager-predict.


****************************
 Build and run main program
****************************
The grader must have a fresh copy of CSPB-3753-Paging-master (https://github.com/CSCI-KNOX/CSPB-3753-Paging)
in order to build and run the paging algorithms included in this file. 

To build pager-lru.c and pager-predict.c, first copy and paste all files within this directory to: 

    ../CSPB-3753-Paging-master/Paging-Simulator/

Overwrite the existing files corresponding to: pager-lru.c, pager-predict.c, and Makefile
    
Next, open a bash shell in the Paging-Simulator directory.

To build both pager-lru.c and pager-predict.c, type "make" in a bash terminal.

To run the least recently used (LRU) paging solution, enter the following in bash:

    ./test-lru

To run the predictive paging solution, enter the following in bash:

    ./test-predict


******************
 Makefile options
******************
The makefile options submitted in this assignment are kept as default. However, in my version
I have more options that may be useful during the grading session if the grader wants to see them. 

These include:

    (1) test: run ./test-predict 10 times, log the results to a file, then compute the average result
    
    (2) matrix: runs the matrix multiplication program used in this assignment


***********************
 Extra credit attempts
***********************
    (1) Extra Credit -- pager-predict.c should have an average score between:
    
        [0.005 <= score < 0.01] for +5 points of extra credit

    
