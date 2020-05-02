/*
 *  The default command line buffer size
 */
#define DEFAULT_BUF   64
#define DEBUG          0

/* 
 *  Global variable: pointer to the head of the jobs list
 */
struct joblist* head = NULL;

/* 
 *  Shell helper functions prototypes
 */
void welcome_msg(void);
void display_prompt(void);
int check_path(char* path);
void sigchld_handler(int sig);
void change_dir(int argc, char* argv[]);
void printArgs(char* argv[], int argc);
int evaluate(char* cmdline, char** args);
char* get_command(char* cmdline, FILE* in_stream, u_int64_t size);
void waitfg(pid_t pid, struct joblist* head, struct joblist* job);
int builtin(int argc, char* argv[], char* cmdline);
int parse(char *cmdline, int* argc, char *argv[], int* pipe_count, int* nextArg);
int checkArg(char* cmdline, char* argv[], int argIdx, int* resized, int* alloc_idx, int* i);
