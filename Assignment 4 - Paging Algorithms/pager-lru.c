/*
 * File: pager-lru.c
 * 
 * Description:
 *     A basic LRU based paging algorithm that follows the stub
 *     provided by the assignment. It creates a blank timestamp 
 *     matrix which contains articfical times (as ticks) when a 
 *     page is referenced. lru() uses these timestamps to select 
 *     the page with the smallest timestamp to evict, because 
 *     this page should be the "least recent"-ly referenced.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "simulator.h"

static int lru(Pentry q, u_int32_t timestamps[], u_int32_t current_tick);

void pageit(Pentry q[MAXPROCESSES]) {

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static u_int32_t timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proc, page, evicted_page;

    /* Initialize static vars on first run */
    if(!initialized){
        for(proc=0; proc < MAXPROCESSES; proc++){
            for(page=0; page < MAXPROCPAGES; page++){
                timestamps[proc][page] = 0;
            }
        }
        initialized = 1;
    }
    /* Load pages requested by each process and into memory */
    for (proc = 0; proc < MAXPROCESSES; proc++) {
        if (q[proc].active) {               // Select an active process
            page = q[proc].pc / PAGESIZE;   // Get the requested page
            timestamps[proc][page] = tick;  // Set a page reference timestamp

            /* Page-in if the page is not in the page table */
            /* Use LRU to select a page to evict if pagein() fails */
            if (!q[proc].pages[page] && !pagein(proc, page)) {    
                evicted_page = lru(q[proc], timestamps[proc], tick);
                pageout(proc, evicted_page);
                pagein(proc, page);                  
            }
        }
    }
    tick++;
}

/* Implementation of LRU: Select a page to evict with the smallest timestamp */
static int lru(Pentry q, u_int32_t timestamps[], u_int32_t current_tick) {
	int page, evicted_page;
	u_int32_t smallest_tick = current_tick;
	for(page = 0; page < MAXPROCPAGES; page++) {
		if(q.pages[page] && timestamps[page] < smallest_tick) {
            smallest_tick = timestamps[page];
            evicted_page = page;
	    }
    }
    return evicted_page;			
}
