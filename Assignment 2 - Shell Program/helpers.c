#define _DEFAULT_SOURCE  // usleep()
#include <stdio.h>       // printf()
#include <stdlib.h>      // realloc()
#include <libgen.h>      // basename()
#include <string.h>      // strchr(), strcmp(), strlen()
#include <unistd.h>      // getcwd(), usleep()
#include <limits.h>      // PATH_MAX for getcwd()
#include "job_list.h"    // jobs list data structure

/////////////////////////////////////////////////////////////////////////////////////
///  Add a job to the jobs list:     
///      -> Add a new job to the job list given its PID and state
///      -> New jobs are added at the front of the jobs list (LIFO)
/////////////////////////////////////////////////////////////////////////////////////
char* get_command(char* cmdline, FILE* in_stream, u_int64_t current_size) {
    
    int next_char, i = 0; 

    /* If the user enters nothing, parse() still requires an initialized command line */
    cmdline[0] = '\0';

    /* Fill the command-line buffer one character at a time */
    while ((next_char = fgetc(in_stream)) != '\n' && next_char != EOF) {

        /* Add the current character to the buffer */
        cmdline[i++] = next_char;

        /* Allocate a bigger buffer if the next character reaches the buffer size */
        if (i == current_size) {
            current_size <<= 3;  // multiply current buffer size by 8
            if ((cmdline = (char *) realloc(cmdline, sizeof(char) * (current_size))) == NULL) {
                perror("[realloc] in get_command");
            }
        }
        cmdline[i] = '\0';  // Null-terminate
    }
    #if DEBUG
    printf("final buffer size: %ld\n", current_size);
    #endif
    return cmdline;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Check whether a given directory path exists by opening and closing the file: 
///      -> Returns 1 if the path exists, 0 if the path does not exist
///      -> Used primarily to implement 'cd'
/////////////////////////////////////////////////////////////////////////////////////
int check_path(char* path) {

    FILE* file_p;

    // Return 0 if the file does not exist
    if ((file_p = fopen(path, "r")) == NULL) {
        return 0;
    }
    // Return 1 if the file exists
    if (fclose(file_p) == EOF) {
        perror("[fclose] in check_path");
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Implementation of 'cd' command to change directory in the shell:     
///      -> Call getcwd() to get the current working directory
///      -> Call strcmp() with argv[1] to check what directory we are moving to
//       -> Call chdir() to move to the new directory
/////////////////////////////////////////////////////////////////////////////////////
void change_dir(int argc, char* argv[]) {
    /* [cd  ]: Return if the user did not enter a new directory */
    if (argc == 1) {
        return;
    }
    /* Get the current working directory */
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("[getcwd] in change_dir");
    }
    /* [cd ..]: Move to the parent directory if it exists */
    if (strcmp(argv[1], "..") == 0) {

        /* Locate the last occurence of '/' and replace it with null terminator */
        char* slash_ptr = strrchr(cwd, '/');
        *slash_ptr = '\0';

        /* If the parent directory exists, go to it */
        if (check_path(cwd)) {
            if (chdir(cwd) == -1) {
                perror("[chdir] in change_dir");
            }
        }
        else {
            printf("cd .. error: parent directory invalid\n");
        }
    }
    /* [cd /dir/] Move to the specified directory if it exists */ 
    if (check_path(argv[1])) {
        if (chdir(argv[1]) == -1) {
            perror("[chdir] in change_dir");
        }
    }
    else {
        printf("cd: %s: no such file or directory\n", argv[1]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///  Busy wait until foreground process completes:     
///      -> Loop blocks user input while a foreground job is running
///      -> Loop exits when the foreground job is removed from the job list
/////////////////////////////////////////////////////////////////////////////////////
void waitfg(pid_t pid, struct joblist* head, struct joblist* job) {
    while (fg_job(head)) {
        usleep(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///  Print command line arguments given to the shell:     
///      -> Loops through the argument vector and prints all command line arguments
///      -> Primarily used for debugging
/////////////////////////////////////////////////////////////////////////////////////
void printArgs(char* argv[], int argc) {
    for (int i=0; i <= argc; i++) {
        printf("argc: %d, argv[%d]: %s\n", argc, i, argv[i]);
    }
    printf("\n");
}

/////////////////////////////////////////////////////////////////////////////////////
///  Display the command line prompt to the user:     
///      -> Escape characters are used change the color of the prompt
/////////////////////////////////////////////////////////////////////////////////////
void display_prompt(void) {
    char buf[PATH_MAX];
    if (getcwd(buf, PATH_MAX) == NULL) {
        perror("[getcwd] in display_prompt");
    }
    char* baseDir = basename(buf);
    printf("\033[1;32m[%s\033[0m\033[1;37m%s\033[0m\033[1;32m%s \033[0m", 
            "shell ", baseDir, "]$"); 
}

/////////////////////////////////////////////////////////////////////////////////////
///  Display a welcome message when the shell first opens:   
///      -> Escape characters are used change the color of the prompt
/////////////////////////////////////////////////////////////////////////////////////
void welcome_msg(void) { 
    printf("\033[1;32m%s\033[0m", " _____                 ____  _          _ _ \n");
    printf("\033[1;32m%s\033[0m", "|_   _|__  _ __ ___   / ___|| |__   ___| | |\n");
    printf("\033[1;32m%s\033[0m", "  | |/ _ \\| '_ ` _ \\  \\___ \\| '_ \\ / _ \\ | |\n");
    printf("\033[1;32m%s\033[0m", "  | | (_) | | | | | |  ___) | | | |  __/ | |\n");
    printf("\033[1;32m%s\033[0m", "  |_|\\___/|_| |_| |_| |____/|_| |_|\\___|_|_|\n\n");                                    
}
