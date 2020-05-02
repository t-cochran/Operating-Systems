/*
 *  File: wrappers.c
 *  
 *  Contents: 
 *    Error handling wrappers for library functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "headers/wrappers.h"

/* 
 *  Print errno and exit
 */
void errno_exit(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/* 
 *  Set errno, print errno, and exit
 */
void errnum_exit(int en, char* msg) {
    errno = en;
    perror(msg);
    exit(EXIT_FAILURE);
}

/* Error check:
 *   int sem_init(sem_t *sem, int pshared, unsigned int value);
 */
void init_semaphore(sem_t* semaphore, int v1, int v2) {
    if (sem_init(semaphore, v1, v2) == -1) {
        errno_exit("sem_init");
    }
}

/* Error check:
 *   int pthread_mutex_init(pthread_mutex_t *restrict mutex,
 *       const pthread_mutexattr_t *restrict attr);
 */
void init_mutex(pthread_mutex_t* mutex) {
    int err = 0;
    if ((err = pthread_mutex_init(mutex, NULL)) > 0) {
        errnum_exit(err, "pthread_mutex_init");
    }
}

/* Error check:
 *   int sem_destroy(sem_t *sem);
 */
void cleanup_semaphore(sem_t semaphore) {
    if (sem_destroy(&semaphore) == -1) {
        perror("sem_destroy");
    }
}

/* Error check:
 *   int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
 *                      void *(*start_routine) (void *), void *arg);
 */
void create_thread(pthread_t* thread, const pthread_attr_t* attr, 
                    void* (*routine)(void *), void* arg) {
    int err = 0;
    if ((err = pthread_create(thread, attr, routine, arg)) != 0) {
        errnum_exit(err, "parser pthread_create");
    }
}

/* Error check:
 *   int pthread_join(pthread_t thread, void **retval);
 */
void join_thread(pthread_t thread, void** ret) {
    int err = 0;
    if ((err = pthread_join(thread, ret)) != 0) {
        errnum_exit(err, "pthread_join");
    }
}

/* Error check:
 *   int pthread_mutex_destroy(pthread_mutex_t *mutex);
 */
void cleanup_mutex(pthread_mutex_t mutex) {
    int err = 0;
    if ((err = pthread_mutex_destroy(&mutex)) != 0) {
        fprintf(stderr, "pthread_mutex_destroy: %d", err);
    }
}

/* Error check:
 *   int fclose (FILE *stream);
 */
void close_file(FILE* fd) {
    if (fclose(fd) == EOF) {
        perror("fclose");
    }
}

/* Error check:
 *   int pthread_mutex_lock(pthread_mutex_t *mutex);
 */
void mutex_lock(pthread_mutex_t* mutex) {
    if (pthread_mutex_lock(mutex) != 0) { 
        errno_exit("pthread_mutex_lock");
    }
}

/* Error check:
 *   int pthread_mutex_unlock(pthread_mutex_t *mutex);
 */
void mutex_unlock(pthread_mutex_t* mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        errno_exit("pthread_mutex_lock");
    }
}

/* Error check:
 *   int sem_wait(sem_t *sem);
 */
void wait_semaphore(sem_t* semaphore) {
    if (sem_wait(semaphore) == -1) {  
        errno_exit("sem_wait");
    }
}

/* Error check:
 *   int sem_post(sem_t *sem);
 */
void signal_semaphore(sem_t* semaphore) {
    if (sem_post(semaphore) == -1) { 
        errno_exit("sem_post");
    }
}

/* Error check:
 *   FILE* fopen (const char *filename, const char *mode);
 */
FILE* open_file(char* path, char* rw) {
    FILE* fd;
    if ((fd = fopen(path, rw)) == NULL) {
        errno_exit("fopen");
    }
    return fd;
}
