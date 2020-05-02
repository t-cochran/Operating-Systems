/*
 *  File: helpers.c
 *  
 *  Contents: 
 *    Helper function definitions used by multi-lookup.c.
 *    Global data definitions accessed by threads created from multi-lookup.c
 */
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "headers/helpers.h"
#include "headers/wrappers.h"

/* 
 *  Define global data
 */
pthread_t parser_threads[MAX_PARSER_THREADS];      // Parser thread IDs
pthread_t converter_threads[MAX_CONVERT_THREADS];  // converter thread IDs
pthread_mutex_t p_log_mutex, c_log_mutex, stack;   // Mutexes
sem_t file_list, producer, consumer;               // Semaphores
FILE* parser_log, *converter_log;                  // Log files
bool parser_done = false;                          // Parser thread status
stack_ds shared_buffer;                             // Stack data structure
f_list files;                                      // Open file list data structure

/***************************************************************
 *  Function:  initialize
 *  ----------------------------------------
 *   argv: command line argument vector.
 * 
 *   Description:
 *     Initializes data structures, semaphores and mutexes
 *     used by multi-lookup.c. Opens thread log files.
 * 
 *   returns:
 *     none
 ***************************************************************/
void initialize(char* argv[]) {
    /* Initialize the stack */
    init_stack(&shared_buffer);

    /* Initialize open input file list */
    init_file_list(&files, argv);  

    /* Initialize semaphores */
    init_semaphore(&producer, 0, MAX_STACK_SIZE); 
    init_semaphore(&consumer, 0, 0);
    init_semaphore(&file_list, 0, 1);

    /* Initialize mutexes */
    init_mutex(&p_log_mutex); 
    init_mutex(&c_log_mutex); 
    init_mutex(&stack);

    /* Open log files */
    parser_log = open_file(argv[3], "w");
    converter_log = open_file(argv[4], "w");
}

/***************************************************************
 *  Function:  init_file_list
 *  ----------------------------------------
 *   files: Pointer to a struct containing open input files.
 *    argv: command line argument vector.
 * 
 *   Description:
 *     Initialize attributes of the file list (f_list) struct.
 * 
 *   returns:
 *       none
 ***************************************************************/
void init_file_list(f_list* files, char** argv) {
    files -> open_files = file_count(argv);
    files -> current_file_idx = 0;
    // Error check: Requested input file count exceeds maximum
    if (files -> open_files > MAX_INPUT_FILES) {
        fprintf(stderr, "\nError: %d files exceeds %d file maximum\n", 
                files -> open_files, MAX_INPUT_FILES);
        exit(0);
    }
    // Add open file descriptors to the file list
    for (int i=0; i < files->open_files; i++) {
        files->fd[i] = fopen(argv[i + 5], "r");
    }
}

/***************************************************************
 *  Function:  cleanup
 *  ----------------------------------------
 *   Description:
 *     Free memory allocated to data structures, destroy
 *     semaphores and mutexes, and close open files.
 * 
 *   returns:
 *     none
 ***************************************************************/
void cleanup() {
    /* Free memory allocated to the stack */
    free_stack(&shared_buffer);

    /* Destroy mutexes */
    cleanup_mutex(p_log_mutex); 
    cleanup_mutex(c_log_mutex);
    cleanup_mutex(stack);

    /* Destroy semaphores */
    cleanup_semaphore(file_list);
    cleanup_semaphore(producer);
    cleanup_semaphore(consumer);

    /* Close log files and input files */
    close_file(parser_log);
    close_file(converter_log);
    close_file_list(&files);
}

/***************************************************************
 *  Function:  readline
 *  ----------------------------------------
 *   files: Pointer to a struct containing open input files.
 *    line: Character array filled with a line read.
 * 
 *   Description:
 *     Loop through open input files one at a time, read
 *     lines until EOF, then go to the next open file. 
 *     current_file_indx tracks which open file is currently
 *     being read from.
 * 
 *   returns:
 *       1 : a line is successfully read from an input file.
 *       0 : no input files are left to read.
 ***************************************************************/
int readline(f_list* files, char line[]) {

    wait_semaphore(&file_list);  // Lock access to the inputer files

    char* domain_name = NULL;
    int total_input_files = files -> open_files;    
    int current_file_index = files -> current_file_idx;

    /* Get a pointer to the current open file */
    FILE* selected = files -> fd[current_file_index];

    /* Select an input file that is not EOF and read a line from it */
    while (current_file_index < total_input_files) {
        /* Not EOF: read a line */
        if (fgets(line, MAX_NAME_LENGTH, selected) && !feof(selected)) {
            domain_name = get_domain(line);  // Get a domain name token  
            memcpy(line, domain_name, MAX_NAME_LENGTH);
            signal_semaphore(&file_list);    // Unlock access to the input files
            return 1;
        }
        else {
            /* EOF: Go to the next input file */
            current_file_index++;
            selected = files -> fd[current_file_index];
            files -> current_file_idx = current_file_index;
        }
    }
    /* No files left with lines to read  */
    signal_semaphore(&file_list);  // Unlock access to the input files
    return 0; 
}

/***************************************************************
 *  Function:  add_parser_log_entry
 *  ----------------------------------------
 *            fd: Parser log file descriptor.
 *         files: Input file list (f_list) struct.
 *   served_list: Array of lines read per input file.
 *           tid: Parser thread ID.
 * 
 *   Description:
 *     Compute the number of lines read and files served,
 *     then write an entry to the parser log file.
 * 
 *   returns:
 *      none
 ***************************************************************/
void add_parser_log_entry(FILE* fd, f_list files, int* served_list, pthread_t tid) {
    
    mutex_lock(&p_log_mutex);  // Lock access to the parser log file

    int files_served = 0, lines_read = 0;

    // Use the served_list to compute the number of lines read and files accessed
    compute_files_served(served_list, files.open_files, &files_served, &lines_read);
    
    // Add an entry to the parser log file
    fprintf(fd, "Thread <%ld> read %d lines from %d file(s).\n", 
                tid % 1000, lines_read, files_served);
    fflush(fd);

    mutex_unlock(&p_log_mutex);  // Unlock access to the parser log file
}

/***************************************************************
 *  Function:  add_converter_log_entry
 *  ----------------------------------------
 *      fd: Converter log file descriptor.
 *   dname: Pointer to a domain name.
 *     tid: Converter thread ID.
 * 
 *   Description:
 *     Collect up to five IP addresses associated with a
 *     domain name, and then record the results in a log file.
 * 
 *   returns:
 *      none
 ***************************************************************/
void add_converter_log_entry(FILE* fd, char* dname) {

    mutex_lock(&c_log_mutex);  // Lock access to the log file

    ip_address* ip_strings = NULL;
    int ip_resolved = 0;

    /* Allocate and fill an array of IP address strings */
    ip_strings = calloc((MAX_IP_ADDRESSES << 3), sizeof(*ip_strings));
    ip_resolved = get_ip_address(dname, ip_strings);

    /* Converter thread resolved a domain name */
    if (ip_resolved && ip_strings) { 
        printf("%s", dname);
        fprintf(fd, "%s", dname);
        for (int i=0; i < MAX_IP_ADDRESSES; i++) {
            if ((int) *ip_strings[i] != 0) {
                fprintf(fd, ", %s", ip_strings[i]);
                printf(", %s", ip_strings[i]);
            }
        }
        printf("\n");
        fprintf(fd, "\n");
    }
    /* Converter thread could not resolve a domain name */
    else {
        fprintf(fd, "%s,\n", dname);
        printf("%s, \n", dname);
    }
    fflush(fd);
    free(ip_strings);  // Free the array of IP address strings

    mutex_unlock(&c_log_mutex);  // Unlock access to the log file
}

/***************************************************************
 *  Function:  check_cmdline
 *  ----------------------------------------
 *       argc: command line argument count.
 *       argv: command line argument vector.
 *   numParse: number of parser threads selected.
 *    numConv: number of converter threads selected.
 * 
 *   Description:
 *     Check the commandline arguments to ensure they are valid.
 *     Print an error and exit if incorrect arguments are given.
 * 
 *   returns:
 *      none
 ***************************************************************/
void check_cmdline(int argc, char** argv, int* numParse, int* numConv) {
 
    /* Check if the minimum number of command-line arguments were entered */
    if (argc < 6 || check_path(argv[1]) || check_path(argv[2]) || 
                    atoi(argv[1]) < 0 || atoi(argv[2]) < 0) {
        fprintf(stderr, "\nUsage: ./multi-lookup <# parsing threads> <# conversion threads>\n");
        fprintf(stderr, "\t<parsing log> <converter log> [ <data file>...]\n\n");
        exit(1);
    }
    /* Check if the number of input files exceeds the maximum */
    if (file_count(argv) > MAX_INPUT_FILES) {
        fprintf(stderr, "\nExceeded the maximum (n = %d) input files.\n\n", 
                MAX_INPUT_FILES);
        exit(1);
    }
    /* Check if the number of converter threads exceeds maximum */
    if (atoi(argv[2]) > MAX_CONVERT_THREADS) {
        fprintf(stderr, "\nExceeded the maximum (n = %d) converter threads.\n\n", 
                MAX_CONVERT_THREADS);
        exit(1);
    }
    /* Check if the number of parser threads exceeds maximum */
    if (atoi(argv[1]) > MAX_PARSER_THREADS) {
        fprintf(stderr, "\nExceeded the maximum (n = %d) parser threads.\n\n", 
                MAX_PARSER_THREADS);
        exit(1);
    }
    /* During serial execution, check whether the stack is large enough */
    if ((atoi(argv[1]) == 0 && atoi(argv[2]) == 0) ||
        (atoi(argv[1]) == 0 && atoi(argv[2]) == 1) ||
        (atoi(argv[1]) == 1 && atoi(argv[2]) == 0)) {
        if (MAX_STACK_SIZE < 100) {
            fprintf(stderr, "\nCaution: Stack size is %d.\n", MAX_STACK_SIZE);
            fprintf(stderr, "You are about to run serial execution ");
            fprintf(stderr, "with %d parsers and %d converters.\n\n", 
                    atoi(argv[1]), atoi(argv[2]));
            fprintf(stderr, "Allocate more stack space in headers/DS_stack.h\n\n");
            exit(1);
        }
    }
    /* Get the command line arguments for the number of: parsers, converters, files */
    *numParse = atoi(argv[1]);    
    *numConv = atoi(argv[2]);  
}

/***************************************************************
 *  Function:  check_cmdline
 *  ----------------------------------------
 *     sec_1: First time reading in seconds.
 *   micro_1: First time reading in microseconds.
 *     sec_2: Second time reading in seconds.
 *   micro_2: Second time reading in microseconds.
 * 
 *   Description:
 *     Compute and print running time for ./multi-lookup
 * 
 *   returns:
 *      none
 ***************************************************************/
void timelapse(long* sec_1, long* micro_1, long* sec_2, long* micro_2) {

    time_t total_ms;
    struct timeval timestamp_1;

    /* Get a timestamp */
    if (gettimeofday(&timestamp_1, NULL) == -1) {
        perror("gettimeofday");
    }
    /* Record the first time reading sec_1 and micro_1 */
    if (!sec_2 && !micro_2) {
        *sec_1 = timestamp_1.tv_sec;
        *micro_1 = timestamp_1.tv_usec;
    }
    /* Record the second time reading sec_2 and micro_2 then compute runtime */
    else if (sec_2 && micro_2) {
        *sec_2 = timestamp_1.tv_sec;
        *micro_2 = timestamp_1.tv_usec;
        total_ms = (((*sec_2 - *sec_1) * 1000000) + (*micro_2 - *micro_1));
        printf("\nRuntime: %f\n", (float)total_ms / 1000000);
    }
    /* Error if timelapse arguments are passed incorrectly */
    else {
        fprintf(stderr, "Error: incorrect timelapse() args.");
        exit(1);
    }
}

/***************************************************************
 *  Function:  get_domain
 *  ----------------------------------------
 *   line: A line of text read from an input file.
 * 
 *   Description:
 *      Tokenizes the line using spaces as delimiters.
 *      Finds the first location of '.' to locate a domain name.
 * 
 *   returns:
 *      (char*) : Pointer to the domain name token.
 *         NULL : A domain name was not found.
 ***************************************************************/
char* get_domain(char* line) {

    char *line_ptr = line, *token = NULL, *newline = NULL;

    while((token = strsep(&line_ptr, " ")) != NULL) {
        if (strchr(token, '.') != NULL) {
            break;
        }
    }
    if (token != NULL) {
        if ((newline = strchr(token, '\n'))) {
            *newline = '\0';
        }
        return token;
    }
    return NULL;
}

/***************************************************************
 *  Function:  get_ip_address
 *  ----------------------------------------
 *   hostname: Domain name string.
 *     ipstrs: Array of ip address strings.
 * 
 *   Description:
 *     Wrapper for dnslookup; fills the array 'ipstrs' with
 *     ip address strings collected by dnslookup.
 * 
 *   returns:
 *      1 : IP addresses were resolved by dnslookup
 *      0 : Could not resolve an IP address
 ***************************************************************/
int get_ip_address(const char* hostname, ip_address* ipstrs) {
    if (dnslookup(hostname, ipstrs) == -1) {
        fprintf(stderr, "Error: Domain name %s could not be resolved.\n", hostname);
        return 0;
    } 
    return 1;
}

/***************************************************************
 *  Function:  init_served_list
 *  ----------------------------------------
 *    arr: An integer array.
 *   size: Size of the integer array passed.
 * 
 *   Description:
 *     Initializes an integer array to zero. Called by parser 
 *     threads to create an array to track files served.
 * 
 *   returns:
 *      none
 ***************************************************************/
void init_served_list(int* arr, int size) {
    for (int i=0; i < size; i++) {
        arr[i] = 0;
    }
}

/***************************************************************
 *  Function:  compute_files_served
 *  ----------------------------------------
 *     arr: An integer array.
 *    size: Size of the integer array passed.
 *   files: Integer pointer for the number of files served.
 *   lines: Integer pointer for the number of lines read.
 * 
 *   Description:
 *     Given an array whose indices correspond to the number of
 *     files served, and values correspond to the number of lines
 *     read, this function is called by parser threads to compute 
 *     the total lines read and the number of files served.
 * 
 *   returns:
 *      none
 ***************************************************************/
void compute_files_served(int* arr, int size, int* files, int* lines) {
    for (int i=0; i < size; i++) {
        *lines += arr[i];
        if (arr[i]) {
            *files += 1;
        }
    }
}

/***************************************************************
 *  Function:  close_file_list
 *  ----------------------------------------
 *   files: Pointer to a struct containing open input files.
 * 
 *   Description:
 *     Closes all open input files within the f_list struct.
 * 
 *   returns:
 *      none
 ***************************************************************/
void close_file_list(f_list* files) {
    int open_files = files -> open_files;
    for (int i=0; i < open_files; i++) {
        fclose(files->fd[i]);
    }
}

/***************************************************************
 *  Function:  file_count
 *  ----------------------------------------
 *   arg: command line argument vector.
 * 
 *   Description:
 *     Compute the total number of input files entered.
 * 
 *   returns:
 *      (int) file_count : the number of input files.
 ***************************************************************/
int file_count(char** arg) {
    int i=5, file_count = 0;
    char* current_file = arg[i];
    while(current_file != NULL) {
        file_count++;
        current_file = arg[++i];
    }
    return file_count;
}

/***************************************************************
 *  Function:  check_path
 *  ----------------------------------------
 *   path: A file path string.
 * 
 *   Description:
 *     Checks whether a given file path exists.
 * 
 *   returns:
 *      1 : The file path exists.
 *      0 : The file path does not exist.
 ***************************************************************/
int check_path(char* path) {
    FILE* file_p;
    if ((file_p = fopen(path, "r")) == NULL) {
        return 0;
    }
    if (fclose(file_p) == EOF) {
        perror("fclose in check_path");
    }
    return 1;  
}