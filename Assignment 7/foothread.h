#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/sched.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define wait(s) semop(s, &pop, 1)               // P(s) or wait(s)
#define signal(s) semop(s, &vop, 1)             // V(s) or signal(s)

struct sembuf pop = {0, -1, 0};                 // struct to perform wait operation
struct sembuf vop = {0, 1, 0};                  // struct to perform signal operation

#define FOOTHREAD_JOINABLE 0                    // joinable thread
#define FOOTHREAD_DETACHED 1                    // detached thread

#define FOOTHREAD_THREADS_MAX 101               // maximum number of threads is 100, 1 is for the main thread
#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152    // default stack size is 2MB

// default initializer for the thread attributes
#define FOOTHREAD_ATTR_INITIALIZER {FOOTHREAD_DEFAULT_STACK_SIZE, FOOTHREAD_DETACHED}

// structure for the table storing the threads information
typedef struct table_t
{
    pid_t tid;
    int stacksize;
    int jointype;
    pid_t ptid;
    int child;
} table_t;

// structure for the threads
typedef struct foothread_t
{
    pid_t tid;
    int stacksize;
    int jointype;
} foothread_t;

// structure for the thread attributes
typedef struct foothread_attr_t
{
    int stacksize;
    int jointype;
} foothread_attr_t;

// structure for the mutex
typedef struct foothread_mutex_t
{
    pid_t tid;
    int mutid;
} foothread_mutex_t;

// structure for the barrier
typedef struct foothread_barrier_t
{
    int count;
    int max;
    int semid;
    int mutid;
} foothread_barrier_t;

// global variables
int num_threads = 0;                    // number of threads currently assigned
int barrierkey = 5;                     // key for the barrier, will be incremented for each barrier
int mutexkey = 105;                     // key for the mutex, will be incremented for each mutex 

// table of threads
table_t threads[FOOTHREAD_THREADS_MAX];

// function prototypes for thread creation, initialization, and exit

void foothread_create(foothread_t *, foothread_attr_t *, int (*)(void *), void *);
void foothread_attr_setjointype(foothread_attr_t *, int);
void foothread_attr_setstacksize(foothread_attr_t *, int);
void foothread_exit(void);

// function prototypes for mutex

void foothread_mutex_init(foothread_mutex_t *);
void foothread_mutex_lock(foothread_mutex_t *);
void foothread_mutex_unlock(foothread_mutex_t *);
void foothread_mutex_destroy(foothread_mutex_t *);

// function prototypes for barrier

void foothread_barrier_init(foothread_barrier_t *, int);
void foothread_barrier_wait(foothread_barrier_t *);
void foothread_barrier_destroy(foothread_barrier_t *);