#include <sys/wait.h>     // pid_t
#include <stdlib.h>       // malloc(), free()
#include <stdio.h>        // printf()
#include "job_list.h"     // jobs list prototypes

/////////////////////////////////////////////////////////////////////////////////////
///  Initializes the joblist:     
///      -> Create the head of the jobs list and return a pointer to it
/////////////////////////////////////////////////////////////////////////////////////
struct joblist* init_joblist(void) {

    /* Dynamically allocate a new job as the head of the list */
    struct joblist* new_job;
    
    if ((new_job = malloc(sizeof(struct joblist))) == NULL) {
        perror("[malloc] in init_joblist");
    }

    /* Set attributes unique to the head of the job list*/
    new_job -> next = NULL;
    new_job -> pid = -1;
    new_job -> state = -1;

    /* Return pointer to the head of the jobs list */
    return new_job;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Add a job to the jobs list:     
///      -> Add a new job to the job list given its PID and state
///      -> New jobs are added at the front of the jobs list (LIFO)
/////////////////////////////////////////////////////////////////////////////////////
struct joblist* add_job(struct joblist* head, pid_t pid, int state) {

    /* Dynamically allocate a new job */
    struct joblist* new_job;
    
    if ((new_job = malloc(sizeof(struct joblist))) == NULL) {
        perror("malloc");
    }
    
    /* set attributes of the added job */
    new_job -> pid = pid;
    new_job -> state = state;

    /* Add the new job to the front of the job list */
    new_job -> next = head -> next;
    head -> next = new_job;

    /* Return pointer to the added job */
    return new_job;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Search for a job in the jobs list given its PID  
///      -> Returns a pointer to the job if it exists
///      -> Called by SIGCHLD handler to identify a completed background job
/////////////////////////////////////////////////////////////////////////////////////
struct joblist* find_job(struct joblist* head, pid_t pid) {
    struct joblist* cursor = head;
    for (; cursor != NULL; cursor = cursor -> next) {
        if ((cursor -> pid) == pid) {
            return cursor;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Print all jobs in the jobs list:     
///      -> Loop through every job in the jobs list and print their PID's
/////////////////////////////////////////////////////////////////////////////////////
void print_jobs(struct joblist* head) {
    /* Loop through the jobs list and print pid's */
    struct joblist* cursor = head -> next;
    if (cursor == NULL) {
        printf("Jobs list is empty.\n");
    }
    else {
        printf("Current jobs:\n");
        for (; cursor != NULL; cursor = cursor -> next) {
            printf("PID of job: [%d]\n", cursor -> pid);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///  Return a pointer to the foreground job in the jobs list:     
///      -> Used primarily by waitfg() to exit the busy loop once the 
///         foreground job has completed  
/////////////////////////////////////////////////////////////////////////////////////
struct joblist* fg_job(struct joblist* head) {
    /* Loop through the jobs and find the foreground job */
    struct joblist* cursor = head;
    for (; cursor != NULL; cursor = cursor -> next) {
        if (cursor -> state == FOREGROUND) {
            return cursor;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Delete a job from the jobs list given its PID :     
///      -> Loop through the jobs list to find the job to delete
///      -> Free the deleted job's memory and fix jobs list pointers
/////////////////////////////////////////////////////////////////////////////////////
int delete_job(struct joblist* head, pid_t pid) {
    
    /* Pointers to assist finding and removing the job */
    struct joblist* prev_job = head;              // pointer to previous job
    struct joblist* deleted_job = head -> next;   // pointer to current job

    /* The job is not the first in the list, so find it */
    for (; deleted_job != NULL; deleted_job = deleted_job -> next) {

        /* Job is found */
        if (deleted_job -> pid == pid) {

            /* Job is at the end of the list*/
            if (deleted_job -> next == NULL) {
                prev_job -> next = NULL;
                free(deleted_job);
                return 0;
            }
            /* Job is in the middle of the list */
            else {
                prev_job -> next = deleted_job -> next;
                free(deleted_job);
                return 0;
            }
        }
        prev_job = deleted_job; // update the prev_job cursor
    }
    return 0;
}
