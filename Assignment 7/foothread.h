#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
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

#define P(s) semop(s, &pop, 1)  // P(s) or wait(s)
#define V(s) semop(s, &vop, 1)  // V(s) or signal(s)

struct sembuf pop = {0, -1, 0};
struct sembuf vop = {0, 1, 0};

#define FOOTHREAD_JOINABLE 0
#define FOOTHREAD_DETACHED 1

#define FOOTHREAD_THREADS_MAX 20
#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152
#define FOOTHREAD_ATTR_INITIALIZER {FOOTHREAD_DEFAULT_STACK_SIZE, FOOTHREAD_DETACHED}

typedef struct table_t
{
    pid_t tid;
    int stacksize;
    int jointype;
    pid_t ptid;
    int semid;
    int child;
} table_t;

typedef struct foothread_t
{
    pid_t tid;
    int stacksize;
    int jointype;
} foothread_t;

typedef struct foothread_attr_t
{
    int stacksize;
    int jointype;
} foothread_attr_t;

typedef struct
{
    pid_t tid;
    int mutid;
} foothread_mutex_t;

typedef struct
{
    int count;
    int max;
    int semid;
    int mutid;
} foothread_barrier_t;

int num_threads = 0;

// table of threads
table_t threads[FOOTHREAD_THREADS_MAX];

void foothread_create(foothread_t *, foothread_attr_t *, int (*)(void *), void *);
void foothread_attr_setjointype(foothread_attr_t *, int);
void foothread_attr_setstacksize(foothread_attr_t *, int);
void foothread_exit();

void foothread_mutex_init(foothread_mutex_t *);
void foothread_mutex_lock(foothread_mutex_t *);
void foothread_mutex_unlock(foothread_mutex_t *);
void foothread_mutex_destroy(foothread_mutex_t *);

void foothread_barrier_init(foothread_barrier_t *, int);
void foothread_barrier_wait(foothread_barrier_t *);
void foothread_barrier_destroy(foothread_barrier_t *);