/* 
 *  File: wrappers.h 
 * 
 *  Contents:
 *    Error handling function prototypes
 */
void signal_semaphore(sem_t* semaphore);
void wait_semaphore(sem_t* semaphore);
void mutex_lock(pthread_mutex_t* mutex);
void mutex_unlock(pthread_mutex_t* mutex);
void cleanup_semaphore(sem_t semaphore);
void cleanup_mutex(pthread_mutex_t mutex);
void close_file(FILE* fd);
void join_thread(pthread_t thread, void** ret);
void create_thread(pthread_t* thread, const pthread_attr_t* attr, void* (*routine)(void *), void* arg);
void init_semaphore(sem_t* semaphore, int v1, int v2);
void init_mutex(pthread_mutex_t* mutex);
void errno_exit(char* msg);
void errnum_exit(int en, char* msg);
FILE* open_file(char* path, char* rw);