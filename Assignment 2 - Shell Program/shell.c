#define _POSIX_SOURCE    // Use POSIX functions and definitions
#include <stdio.h>       // prinf()
#include <stdlib.h>      // malloc(), free()  
#include <signal.h>      // SIGCHLD handling
#include <unistd.h>      // fork(), pipe(), dup2(), chdir()
#include <sys/wait.h>    // wait()
#include <errno.h>       // errno
#include <sys/types.h>   // u_int64_t type 
#include <string.h>      // strtok(), strchr(), strlen(), strcat()   
#include <fcntl.h>       // open(), O_RDWR
#include <limits.h>      // PATH_MAX for getcwd()
#include <stdbool.h>     // true, false
#include "job_list.h"    // jobs list data structure
#include "helpers.h"     // shell helper routines

/////////////////////////////////////////////////////////////////////////////////////
///  Shell read-evaluate loop:     
///      -> Display the command-line prompt and set the SIGCHLD handler
///      -> Dynamically allocate the command-line buffer and argument list
///      -> Read the command line and evaluate the arguments
/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

    char *cmdline;                // Pointer to the command-line string
    char** args;                  // Pointers to command-line arguments
    u_int64_t buf = DEFAULT_BUF;  // Set initial buffer size to 64 bytes
    head = init_joblist();        // Initialize the jobs list
    welcome_msg();                // Print a welcome message

    while(1) { 
        /* Display the prompt */
        display_prompt();

        /* Set a SIGCHLD handler to reap child processes */
        if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
            perror("[signal] in main");
        }
        /* Dynamically allocate the command-line buffer and argument vector */
        if ((cmdline = (char *) malloc(sizeof(char) * buf)) == NULL) {
            perror("[malloc] in main");
        }
        if ((args = (char**) malloc(sizeof(char *) * buf)) == NULL) {
            perror("[malloc] in main");
        }
        /* Read the command-line and fill the command line buffer */ 
        cmdline = get_command(cmdline, stdin, buf);

        /* Evaluate the command-line and free buffers when complete */
        if (evaluate(cmdline, args) == 0) {
            free(cmdline);
            free(args);
        }
        /* Set command-line buffer size back to default */
        buf = DEFAULT_BUF; 
        if (fflush(stdout) == EOF) {
            perror("[fflush] in main");
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///  Evaluate the command line by parsing it and executing the arguments:     
///      -> Parse the command line and return an argument vector
///      -> Return immediately if the job is a builtin command
///      -> Check each argument to ensure it is a correct command 
///      -> Create pipes if there are pipes
///      -> Fork child processes to execute commands from the argument vector
///      -> Reap child process that have exited and check their exit status
///      -> Free any arguments that were allocated with malloc() by checkArg()
/////////////////////////////////////////////////////////////////////////////////////
int evaluate(char* cmdline, char** args) {

    pid_t pid;
    struct joblist* added_job; 
    int background_job = 0, pipe_count = 0, argc = 0, alloc_idx = 0, resized_args = 0, 
        read_end = 0, write_end = 1;
    int pipe_fd[20], alloc_array[20], nextArg_idx[20];

    /*  Parse the command line and build the argument list.
     *    Returns Values: (-1) User entered blank line, (0) Foreground job, (1) Background job */
    if ((background_job = parse(cmdline, &argc, args, &pipe_count, nextArg_idx)) == -1) { 
        return 0;
    }
    /* Check if the command is built-in */
    if (builtin(argc, args, cmdline)) {
        return 0;
    }
    /* Checks if the argv's are valid and prepended with /bin/ */
    for (int i = 0; i <= pipe_count; i++) {  
        if ((checkArg(cmdline, args, nextArg_idx[i], &resized_args, alloc_array, 
                      &alloc_idx)) == -1) {
            return 0;
        }
    }
    #if DEBUG 
    printArgs(args, argc);
    #endif

    /* If the user uses pipe(s), create the pipe(s) */
    if (pipe_count) { 
        for (int i=0; i < pipe_count * 2; i+=2) {
            if (pipe(pipe_fd + i) == -1) {
                perror("[pipe] in evaluate");
            }
        }
    }
    /* Create child processes to exec() commands from the argument list */
    switch(pid = fork()) {
        case -1:
            perror("[fork] in evaluate");
        case 0:  /* Child process *************************************************/
            setpgid(0, 0);
            if (pipe_count) {
                /* Close the unused read end of the pipe */
                if (close(pipe_fd[read_end]) == -1) {
                    perror("[close pipe] in evaluate");
                }
                /* dup2() will replace the write end of the pipe with stdout */
                if (dup2(pipe_fd[write_end], STDOUT_FILENO) == -1) {
                        perror("[dup2] in evaluate");
                }
            }
            /* exec() the child proces */
            if (execvp(args[nextArg_idx[0]], args + (nextArg_idx[0])) == -1) {
                perror("[execvp] in evaluate");
            } 
    }
    /* Parent process ********************************************************/
    /* If there are pipes, fork() a second time to exec() the other end of the pipe */
    if (pipe_count) {
        switch(pid = fork()) {
            case -1:
                perror("fork");
            case 0:  /** Child process ******************************************/
                setpgid(0, 0);
                /* Close the unused write end of the pipe */
                if (close(pipe_fd[write_end]) == -1) {
                    perror("[close pipe] in evaluate");
                }
                /* dup2() will replace the read end of the pipe with stdin */
                if (dup2(pipe_fd[read_end], STDIN_FILENO) == -1) {
                    perror("[dup2] in evaluate");
                }
                /* Child process loads its arguments and executes */
                if (execvp(args[nextArg_idx[1]], args + (nextArg_idx[1])) == -1) {  
                    perror("[execvp] in evaluate");
                }
            default: /* Parent process *****************************************/
                /* Wait for child to complete before closing the pipes */
                wait(NULL);
                /* Close the pipes */
                if (close(pipe_fd[read_end]) == -1) {
                    perror("[close pipe] in evaluate");
                }
                if (close(pipe_fd[write_end]) == -1) {
                    perror("[close pipe] in evaluate");
                }
        }
    }
    /* Parent process ********************************************/
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("[signal] in evaluate");
    }
    /* Foreground job: Call waitfg() to suspend execution until complete */
    if (!background_job) { 
        added_job = add_job(head, pid, FOREGROUND); 
        waitfg(pid, head, added_job);
    }
    /* Background job: Do not suspend execution */
    if (background_job) {
        added_job = add_job(head, pid, BACKGROUND);
    }
    /* Free allocated arguments from the argument list */
    if (resized_args) {
        for (int i = 0; i < alloc_idx; i++) {
            free(args[alloc_array[i]]);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Check command line arguments for:     
///      -> If an argument is not prepended with /bin/, then prepend it with /bin/
///      -> Track allocated arguments so they can be free'd later
///      -> Check access to the /bin/argv[] file to see whether it exists
/////////////////////////////////////////////////////////////////////////////////////
int checkArg(char* cmdline, char* argv[], int argIdx, int* resized, int* alloc_idx, int* i) {

    char binCheck[6];
    char* binConcat = NULL;
    bool prepend = false;

    /* Check for /bin/ by loading the first 5 characters into 'binCheck' */
    sprintf(binCheck, "%.5s", argv[argIdx]); 

    /* If 'bincheck' does not contain /bin/, prepend it */
    if (strcmp(binCheck, "/bin/") != 0) { 

        /* Dynamically allocate a new argument containing /bin/ */
        if ((binConcat = (char *) malloc(sizeof(char) * (6 + strlen(argv[argIdx])))) == NULL) {
            perror("[malloc] in checkArg");
        }
        strcpy(binConcat, "/bin/");

        /* Concatenate argv[] to /bin/ and update argv[] */
        strcat(binConcat, argv[argIdx]);
        argv[argIdx] = binConcat;

        /* Track the allocated arguments so they can be freed later */
        prepend = true;
        alloc_idx[*i] = argIdx;
        (*i)++;
    }
    /* Check access to the file /bin/argv[] */
    if (access(argv[argIdx], F_OK) == -1) {
        printf("shell: %s: command not found\n", argv[argIdx]);
        free(binConcat);
        return -1;
    }
    *resized |= prepend;  // Notifies evaluate() to free allocated arguments
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Parse the command line and build the argument vector:   
///      -> Use strchr() to check if the command is executed in the background.
///      -> Use strtok() to tokenize the command line and build the argument vector.
///      -> Return immediately if there are no arguments (user entered a blank line)
///      -> Turn on colors for 'grep' or 'ls' by inserting --color=auto
///      -> Add a null argument to the end of the argument vector for execvp()
///      -> If a pipe exists: replace it will null and track the location and amount.
/////////////////////////////////////////////////////////////////////////////////////
int parse(char* cmdline, int* argc, char* argv[], int* pipe_count, int* nextArg) {

    /* Check if the command is a background job or foreground job */
    int background_job;
    if ((strchr(cmdline, '&')) != NULL) {
        background_job = true;
    }
    else {
        background_job = false;
    }
    /* Tokenize cmdline and build the argument vector */
    *argc = 0;
    char* pch;
    pch = strtok(cmdline, " ,'&\n");
    while (pch != NULL) {
        argv[*argc] = pch;               // add token to the arg vector
        (*argc)++;                       // increment arg count
        pch = strtok(NULL, " ,'&\n");    // continue scanning
    }
    /* Ignore empty lines or characters that were not delimited by strtok() */
    if (*argc == 0 || *argv[0] == '|' || *argv[0] == '.') {
        return -1;
    }
    /* Insert the argument --color=auto if the user entered 'grep' or 'ls' */
    int pos = 0;
    for (; pos < *argc; pos++) {
        if (strcmp(argv[pos], "grep")      == 0 || 
            strcmp(argv[pos], "ls")        == 0 ||
            strcmp(argv[pos], "/bin/grep") == 0 ||
            strcmp(argv[pos], "/bin/ls")   == 0) {
            for (int i=*argc; i > pos; i--) {  // shift tokens to the right
                argv[i] = argv[i - 1];  
            }
            argv[pos + 1] = "--color=auto\0";  // insert --color=auto
            (*argc)++;
        }
    }
    /* Add a NULL token since exevp() expects a null-terminated arg vector */
    argv[*argc] = NULL;

    /* Check for pipes and if they exist, count them and store their locations */
    int idx = 1; 
    nextArg[0] = 0;
    for (int i=0; i < *argc; i++) {      // loop over all argument tokens 
        if (strcmp(argv[i], "|") == 0) {
            argv[i] = NULL;              // replace pipe tokens with null for execvp()
            nextArg[idx] = i + 1;        // Track the argument index after the pipe
            *pipe_count += 1;            // Track the number of pipes requested
            idx++;
        } else {
            nextArg[idx] = 0;
        }
    }
    #if DEBUG
    printf("\nNumber of pipes detected: %d\n\n", *pipe_count);
    #endif
    return background_job;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Built-in commands that run without creating child processes:
///      -> 'quit' or 'DONE': free pointers to allocated memory and exit the shell.
///      -> 'jobs': Print the current jobs running in the background.
///      -> 'tokenize <string>': Print a tokenized string to the screen.
///      -> 'pwd': Print the current working directory to the screen.
///      -> 'cd <dir>' or 'cd <..>: Change the current working directory
/////////////////////////////////////////////////////////////////////////////////////
int builtin(int argc, char *argv[], char* cmdline) {
    if (strcmp(argv[0], "quit") == 0 || 
        strcmp(argv[0], "DONE") == 0) {
        free(head); free(argv); free(cmdline);
        printf("Exiting shell...\n");
        exit(0);
    }
    if (strcmp(argv[0], "jobs") == 0) {
        print_jobs(head);
        return 1;
    }
    if (strcmp(argv[0], "tokenize") == 0) {
        printf("Printing cmdline tokens... \n");
        for (int i = 1; i < argc; ++i) {
            printf("\"%s\"\n", argv[i]);
        }
        return 1;
    }
    if (strcmp(argv[0], "pwd") == 0) {
        char buf[PATH_MAX];
        if (getcwd(buf, PATH_MAX) == NULL) {
            perror("[getcwd] in builtin");
        }
        printf("%s\n", buf);
        return 1;
    }
    if (strcmp(argv[0], "cd") == 0) {
        change_dir(argc, argv);
        return 1;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Basic SIGCHLD handler to reap child processes:
///      -> Calls waitpid(-1, &status, WNOHANG) options:
///         -> -1 waits for any exiting child process
///         -> &status records the exit status
///         -> WNOHANG returns 0 immediately if there are no children to reap
///      -> WIFEXITED() returns true if the reaped child exited normally
///      -> delete the corresponding job from the jobs list
///      -> Prints the PID and exit status of the terminating process
/////////////////////////////////////////////////////////////////////////////////////
void sigchld_handler(int sig) {

    pid_t pid;    // Process ID of reaped child
    int status;   // Exit status of reaped child
    struct joblist* job;

    /* Reap child processes with waitpid() */
    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /* Check exit status of the child */
        if (WIFEXITED(status)) {
            #if DEBUG
            printf("[PID: %d] Exit normal with status: %d\n", 
                    pid, WIFEXITED(status));
            #endif
            if ((job = find_job(head, pid)) != NULL) {
                if (job -> state == BACKGROUND) {
                    printf("Background job complete. [PID: %d, Exit Status: %d]\n", 
                           pid, status);
                }
                if (job -> state == FOREGROUND) {
                    printf("Foreground job complete. [PID: %d, Exit Status: %d]\n", 
                           pid, status);
                }
            }
            delete_job(head, pid);  // remove from the jobs list
        }
        else {
            perror("[Child did not terminate normally]");
        }
    }
}

