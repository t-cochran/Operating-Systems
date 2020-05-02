/*
 *  File: helpers.h
 *  
 *  Contents: 
 *    Limits, file list struct, global data declarations, helper function prototypes
 *    for multi-lookup.c
 */
#include "DS_stack.h"
#include "util.h"

/* 
 *  Limits
 */
#define MAX_IP_LENGTH          20
#define MAX_INPUT_FILES        50
#define MAX_PARSER_THREADS     100
#define MAX_CONVERT_THREADS    100
#define MAX_IP_ADDRESSES       5

/* 
 *  Attribute to notify compiler of unused parameters
 */
# define UNUSED_PARAM __attribute__((unused))

/* 
 *  File list struct
 */
typedef struct {
    int open_files;
    int current_file_idx;
    FILE* fd[MAX_INPUT_FILES];
} f_list;

/* 
 *  Declared global data
 */
extern sem_t file_list;
extern pthread_mutex_t log_mutex;
extern pthread_t parser_threads[MAX_PARSER_THREADS];
extern pthread_t converter_threads[MAX_CONVERT_THREADS];
extern bool parser_done;
extern sem_t producer, consumer;
extern pthread_mutex_t p_log_mutex, c_log_mutex, stack;
extern FILE* parser_log, *converter_log;
extern stack_ds shared_buffer;
extern f_list files;

/* 
 *  Helper function prototypes
 */
void initialize(char* argv[]);
void init_file_list(f_list* files, char** argv);
void cleanup();
int readline(f_list* files, char line[]);
void add_parser_log_entry(FILE* fd, f_list files, int* served_list, pthread_t tid);
void add_converter_log_entry(FILE* fd, char* dname);
void check_cmdline(int argc, char** argv, int* numParse, int* numConv);
void timelapse(long* sec_1, long* micro_1, long* sec_2, long* micro_2);
int get_ip_address(const char* hostname, ip_address* ipstrs);
void init_served_list(int* arr, int size);
void compute_files_served(int* arr, int size, int* files, int* lines);
void close_file_list(f_list* files);
char* get_domain(char* line);
int file_count(char** arg);
int check_path(char* path);
void* parser_routine(void* arg);
void* converter_routine(void* arg);
