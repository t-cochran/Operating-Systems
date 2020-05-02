/*
 * File: pager-predict.c
 *
 * Description:
 *     A predictive paging algorithm that lowers page-fault 
 *     counts by paging-in pages with a high probability of 
 *     being referenced in the future. The probabilities of 
 *     each page being referenced in the future is pulled
 *     from probability transition matrices in pager-predict.h.
 *   
 * How did I construct the probability transition matrices?
 *     I added code to simulator.c that printed timestamps for 
 *     each page being referenced over the lifetime of a program 
 *     in programs.c. Then, I ran each program in program.c one 
 *     at a time and created a graph of page accesses over time. 
 *     Then, I computed the frequency of page accesses from this 
 *     data, and then the probability that a page will be accessed 
 *     from some other page.
 * 
 * Please refer to cochran_PA4/PA4_README/PAGER_PREDICT_INFO.pdf 
 * for more information on constructing the matrices used in 
 * pager-predict.c 
 *  
 */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>

#include "simulator.h"
#include "pager-predict.h"

void pageit(Pentry q[MAXPROCESSES]) { 

    /* Static vars */
    static int initialized = 0;
    static int ref_bits[15];
    
    /* Local vars */
    int proc, page;

    /* Initialize static vars on first run */
    if(!initialized) {
        for (int i=0; i < MAXPROCESSES; i++) {
            ref_bits[i] = 0;  // init reference bits
        }
        initialized = 1;
    }
    /* Select an active process */
    for(proc=0; proc < MAXPROCESSES; proc++) {
        if (q[proc].active) {
            page = q[proc].pc/PAGESIZE;   // Get the requested page
            ref_bits[page] = 1;           // Add a reference bit for the requested page

            /* Predict future page references by indexing the probability transiton matrices */
            for (int i=0; i < 15; i++) {  
                if (markov_one_step[page][i] >= 0.200 || markov_two_step[page][i] >= 0.170) {
                    ref_bits[i] = 1;  // Add a reference bit for the predicted page
                }
            }

            /* Page-in the requested and predicted pages */
            for (int i=0; i < 15; i++) {
                if (q[proc].pages[i] && ref_bits[i] == 0) {
                    pageout(proc, i);
                }
                if (!q[proc].pages[i] && ref_bits[i] == 1) {
                    pagein(proc, i);
                }
                ref_bits[i] = 0;  // Reset reference bits
            }
        }
    }
} 
