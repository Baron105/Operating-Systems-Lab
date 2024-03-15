#include "foothread.h"

// create a thread with the given attributes using clone
void foothread_create(foothread_t *thread, foothread_attr_t *attr, int (*start_routine)(void *), void *arg)
{ 

    num_threads++;
    if (num_threads > FOOTHREAD_THREADS_MAX)
    {
        printf("Max number of threads reached\n");
        exit(1);
    }

    // if the 2nd param is NULL, use the default values
    if (attr == NULL)
    {
        *attr = (foothread_attr_t) FOOTHREAD_ATTR_INITIALIZER;
    }

    // allocate memory for the stack
    void *stack = malloc(attr->stacksize);
    if (stack == NULL)
    {
        perror("Memory allocation failed\n");
        exit(1);
    }
    void *stacktop = stack + attr->stacksize;

    // set the flags
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM | CLONE_THREAD;

    // if (attr->jointype == FOOTHREAD_DETACHED)
    // {
        // flags |= CLONE_THREAD;
    // }

    // create the thread
    pid_t tid = clone(start_routine, stacktop, flags, arg);

    if (tid == -1)
    {
        perror("Clone failed\n");
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

    // add the thread to the table
    threads[num_threads - 1].tid = tid;
    threads[num_threads - 1].stacksize = attr->stacksize;
    threads[num_threads - 1].jointype = attr->jointype;
    threads[num_threads - 1].ptid = gettid();

    // increment the number of children of the parent thread if the thread is joinable
    if (attr->jointype == FOOTHREAD_JOINABLE)
    {
        for (int i = 0; i < num_threads; i++)
        {
            if (threads[i].tid == gettid())
            {
                threads[i].child++;
                break;
            }
        }
    }

    // create a semaphore for the thread
    threads[num_threads - 1].semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
}

// set jointype in the attribute
void foothread_attr_setjointype(foothread_attr_t *attr, int jointype)
{
    attr->jointype = jointype;
}

// set stacksize in the attribute
void foothread_attr_setstacksize(foothread_attr_t *attr, int stacksize)
{
    attr->stacksize = stacksize;
}

// exit the thread, more of a synchronization function
void foothread_exit(void)
{
    // get the tid of the current thread
    int current_tid = gettid();
    
    for (int i = 0; i < num_threads; i++)
    {
        // if it is same as ptid, it is the leader thread, make a barrier to wait for all children to finish
        if (threads[i].tid == current_tid && threads[i].ptid == current_tid)
        {
            
        }
    }

    // clear the thread from the table
    for (int i = 0; i < num_threads; i++)
    {
        if (threads[i].tid == gettid())
        {
            threads[i].tid = 0;
            threads[i].stacksize = 0;
            threads[i].jointype = 0;
            threads[i].ptid = 0;
            semctl(threads[i].semid, 0, IPC_RMID, 0);
            threads[i].semid = 0;
            threads[i].child = 0;
            num_threads--;
            break;
        }
    }
}

// initialize the mutex
void foothread_mutex_init(foothread_mutex_t *mutex)
{
    mutex->mutid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    // set the semaphore to 1
    semctl(mutex->mutid, 0, SETVAL, 1);
}

// lock the mutex
void foothread_mutex_lock(foothread_mutex_t *mutex)
{
    // wait until the semaphore is available
    wait(mutex->mutid);
    // after the semaphore is available, set the tid of thread that locked the mutex
    mutex->tid = gettid();
}

// unlock the mutex
void foothread_mutex_unlock(foothread_mutex_t *mutex)
{
    // check if the semaphore is unlocked already
    int val;
    val = semctl(mutex->mutid, 0, GETVAL);
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
            signal(mutex->mutid);
        }
    }
}

// destroy the mutex
void foothread_mutex_destroy(foothread_mutex_t *mutex)
{
    // destroy the semaphore
    semctl(mutex->mutid, 0, IPC_RMID, 0);
}

// initialize the barrier
void foothread_barrier_init(foothread_barrier_t *barrier, int max)
{
    barrier->max = max;
    barrier->count = 0;
    barrier->mutid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    semctl(barrier->mutid, 0, SETVAL, 1);
    barrier->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    semctl(barrier->semid, 0, SETVAL, 0);
}

// wait for all threads to reach the barrier
void foothread_barrier_wait(foothread_barrier_t *barrier)
{
    // lock the mutex
    wait(barrier->mutid);
    barrier->count++;
    // unlock the mutex
    signal(barrier->mutid);

    // if all threads have reached the barrier, unlock the semaphore
    if (barrier->count == barrier->max)
    {
        signal(barrier->semid);
    }

    // wait for the semaphore to be unlocked
    wait(barrier->semid);
    // once the thread is unblocked, signal other threads to continue
    signal(barrier->semid);

    // decrement the count
    // wait(barrier->mutid);
    // barrier->count--;
    // signal(barrier->mutid);
}

// destroy the barrier
void foothread_barrier_destroy(foothread_barrier_t *barrier)
{
    // destroy the semaphores
    semctl(barrier->mutid, 0, IPC_RMID, 0);
    semctl(barrier->semid, 0, IPC_RMID, 0);
}
