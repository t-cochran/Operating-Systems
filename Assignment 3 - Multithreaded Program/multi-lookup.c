/*
 *  Programming Assignment 3
 *  CSPB 3753
 * 
 *  Author: Thomas Cochran
 * 
 *  Program description:
 *     This multithreaded program performs DNS lookups by 
 *     reading domain names listed within input files, 
 *     then resolves each domain name to IP addresses. 
 *     Results are written to a log file: 'results.log'
 * 
 *  See the README.md for how to run the program with valid 
 *  commandline arguments.
 */
#include <pthread.h>
#include <semaphore.h>
#include "headers/helpers.h"
#include "headers/wrappers.h"

/***************************************************************
 *  Function:  main
 *  ----------------------------------------
 *   argc: command line argument count
 *   argv: command line argument vector
 * 
 *   Description:
 *      Initializes data structures, mutexes and semaphores.
 *      Creates parser and converter threads to run routines.
 *      Computes and prints program running time.
 *      Deallocates data structures, mutexes and semaphores.
 * 
 *   returns: 0
 ***************************************************************/
int main(int argc, char* argv[]) {

    time_t sec1, sec2, micro1, micro2;
    int num_parsers, num_converters;

    /* Get valid command-line arguments */
    check_cmdline(argc, argv, &num_parsers, &num_converters);

    /* Init data structures, semaphores, and open files */
    initialize(argv);

    /* Create a timestamp */    
    timelapse(&sec1, &micro1, NULL, NULL);

    /* Create parser threads */
    if (!num_parsers) {parser_routine(NULL);}
    for (int i = 0; i < num_parsers; i++) {
        create_thread(&parser_threads[i], NULL, parser_routine, NULL);
    } 
    
    /* Create converter threads */
    for (int i = 0; i < num_converters; i++) {
        create_thread(&converter_threads[i], NULL, converter_routine, NULL);
    }

    /* Join parser threads */
    for (int i=0; i < num_parsers; i++) {    
        join_thread(parser_threads[i], NULL);
    }
    parser_done = true; 

    /* Join converter threads */
    if (!num_converters) {converter_routine(NULL);}
    for (int i=0; i < num_converters; i++) {
        join_thread(converter_threads[i], NULL);
    }

    /* Create a timestamp and print program running time */
    timelapse(&sec1, &micro1, &sec2, &micro2);

    /* Deallocate data structures, semaphores, and close files */
    cleanup();

    return 0;
}

/***************************************************************
 *  Function:  parser_routine
 *  ----------------------------------------
 *   arg: command line argument vector (unused).
 * 
 *   Description:
 *      Routine executed by parser threads. Read lines from
 *      input files containing domain names. Push domain
 *      names onto a stack for converter threads to resolve.
 *      Add a log entry to parser.log containing the parser
 *      thread ID, number of input files accessed, and number
 *      of lines read.
 * 
 *   returns:
 *      NULL
 ***************************************************************/
void* parser_routine(UNUSED_PARAM void* arg) {
    
    char line[MAX_NAME_LENGTH];

    /* Initialize an array to track lines read and input files served */
    int served_list[files.open_files];
    init_served_list(served_list, files.open_files);
    
    /* Read lines from input files and push them to the stack */
    while(readline(&files, line)) {

        wait_semaphore(&producer);    // Parser waits when the stack is full
        mutex_lock(&stack);           // Lock access to the stack
        
        push(&shared_buffer, line);
        served_list[files.current_file_idx] += 1;
        
        mutex_unlock(&stack);         // Unlock access to the stack
        signal_semaphore(&consumer);  // Unblock the converter if it's waiting on an empty stack
    }

    /* Add a parser log entry */
    add_parser_log_entry(parser_log, files, served_list, pthread_self());

    pthread_exit(NULL);
    return NULL;
}

/***************************************************************
 *  Function:  converter_routine
 *  ----------------------------------------
 *   arg: command line argument vector (unused).
 * 
 *   Description:
 *      Routine executed by converter threads. Pop domain
 *      names from a stack and resolve their IP addresses.
 *      Add a log entry to converter.log containing the 
 *      converter thread ID, domain names, and IP addresses.
 * 
 *   returns:
 *      NULL
 ***************************************************************/
void* converter_routine(UNUSED_PARAM void* arg) {

    char current_domain[MAX_NAME_LENGTH];

    /* Pop domain names from the stack */
    while(!parser_done || !is_empty(&shared_buffer)) {     
        wait_semaphore(&consumer);    // Converter waits when the stack is empty
        mutex_lock(&stack);           // Mutex locks access to the stack
        
        pop(&shared_buffer, current_domain);

        mutex_unlock(&stack);         // Mutex unlocks access to the stack
        signal_semaphore(&producer);  // Unblock the parser if its waiting on a full stack

        /* Resolve IP address and add a converter log entry */
        add_converter_log_entry(converter_log, current_domain);
    }

    pthread_exit(NULL);
    return NULL;
}
