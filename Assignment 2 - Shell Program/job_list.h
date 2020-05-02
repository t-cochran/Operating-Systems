/*
 *  Job states: Distinguish a foreground job from a background job
 */
#define FOREGROUND  0
#define BACKGROUND  1

/*
 *  Joblist members:
 *     -> Pointer to the next job in the jobs list
 *     -> Job process identifier (PID)
 *     -> Job state (foreground or background)
 */
struct joblist {

    struct joblist* next;
    pid_t pid;
    int state;

};

/*
 *  Joblist helper function prototypes
 */
struct joblist* init_joblist(void);
struct joblist* add_job(struct joblist* head, pid_t pid, int state);
struct joblist* find_job(struct joblist* head, pid_t pid);
void print_jobs(struct joblist* head);
struct joblist* fg_job(struct joblist* head);
int delete_job(struct joblist* head, pid_t pid);