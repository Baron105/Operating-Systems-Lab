#include "foothread.h"

void foothread_attr_setjointype(foothread_attr_t *attr, int jointype)
{
    attr->jointype = jointype;
}

void foothread_attr_setstacksize(foothread_attr_t *attr, int stacksize)
{
    attr->stacksize = stacksize;
}

void foothread_create(foothread_t *thread, foothread_attr_t *attr, int (*start_routine)(void *), void *arg)
{
    num_threads++;
    if (num_threads > MAX_THREADS)
    {
        printf("Max number of threads reached\n");
        exit(1);
    }

    // allocate memory for the stack
    void *stack = malloc(attr->stacksize);
    if (stack == NULL)
    {
        perror("malloc");
        exit(1);
    }
    void *stacktop = stack + attr->stacksize;

    // if the 2nd param is NULL, use the default values
    if (attr == NULL)
    {
        attr->stacksize = FOOTHREAD_DEFAULT_STACK_SIZE;
        attr->jointype = FOOTHREAD_DETACHED;
    }

    // set the flags
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM;

    if (attr->jointype == FOOTHREAD_DETACHED)
    {
        flags |= CLONE_THREAD;
    }

    // create the thread
    pid_t tid = clone(start_routine, stacktop, flags, arg);

    if (tid == -1)
    {
        perror("clone");
        exit(1);
    }

    // check if thread param is not NULL
    if (thread != NULL)
    {
        // set the values of the thread
        thread->tid = tid;
        thread->stacksize = attr->stacksize;
        thread->jointype = attr->jointype;
    }
}

void foothread_mutex_init(foothread_mutex_t *mutex)
{
    sem_init(&mutex->sem, 0, 1);
}

void foothread_mutex_lock(foothread_mutex_t *mutex)
{
    // wait until the semaphore is available
    sem_wait(&mutex->sem);
    // after the semaphore is available, set the tid of thread that locked the mutex
    mutex->tid = gettid();
}

void foothread_mutex_unlock(foothread_mutex_t *mutex)
{
    // check if the semaphore is unlocked already
    int val;
    sem_getvalue(&mutex->sem, &val);
    if (val > 0)
    {
        printf("Mutex is already unlocked\n");
        exit(1);
    }
    else 
    {
        // check if the thread that is unlocking the mutex is the same as the one that locked it
        if (mutex->tid != gettid())
        {
            printf("Thread %d is not the owner of the mutex\n", gettid());
            exit(1);
        }
        else
        {
            // unlock the semaphore
            sem_post(&mutex->sem);
        }
    }
}

void foothread_mutex_destroy(foothread_mutex_t *mutex)
{
    // destroy the semaphore
    sem_destroy(&mutex->sem);
}

void foothread_barrier_init(foothread_barrier_t *barrier, int max)
{
    barrier->max = max;
    barrier->count = 0;
    foothread_mutex_init(&barrier->mut);
    sem_init(&barrier->sem, 0, 0);
}

void foothread_barrier_wait(foothread_barrier_t *barrier)
{
    foothread_mutex_lock(&barrier->mut);
    barrier->count++;
    if (barrier->count == barrier->max)
    {
        barrier->count = 0;
        for (int i = 0; i < barrier->max - 1; i++)
        {
            sem_post(&barrier->sem);
        }
        foothread_mutex_unlock(&barrier->mut);
        return;
    }
    foothread_mutex_unlock(&barrier->mut);
    sem_wait(&barrier->sem);
}

void foothread_barrier_destroy(foothread_barrier_t *barrier)
{
    sem_destroy(&barrier->sem);
    foothread_mutex_destroy(&barrier->mut);
}
