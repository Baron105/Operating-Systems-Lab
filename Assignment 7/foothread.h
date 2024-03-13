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
#include <semaphore.h>
#include <errno.h>

#define FOOTHREAD_ATTR_INITIALIZER {FOOTHREAD_DEFAULT_STACK_SIZE, FOOTHREAD_DETACHED}
#define FOOTHREAD_THREADS_MAX 20
#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152

#define FOOTHREAD_JOINABLE 0
#define FOOTHREAD_DETACHED 1

typedef struct
{
    pid_t tid;
    int stacksize;
    int jointype;
    pid_t ptid;
    sem_t mutex;
} table_t;

typedef struct
{
    pid_t tid;
    int stacksize;
    int jointype;
} foothread_t;

typedef struct
{
    int stacksize;
    int jointype;
} foothread_attr_t;

typedef struct
{
    pid_t tid;
    sem_t sem;
} foothread_mutex_t;

typedef struct
{
    int count;
    int max;
    sem_t sem;
    sem_t mut;
} foothread_barrier_t;

int num_threads = 0;

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