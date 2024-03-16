#include "foothread.h"

// create a thread with the given attributes using clone
void foothread_create(foothread_t *thread, foothread_attr_t *attr, int (*start_routine)(void *), void *arg)
{
    key_t key = ftok("foothread.c", 1);
    int tmutex = semget(key, 1, 0666 | IPC_CREAT);
    key = ftok("foothread.c", 2);
    int tsem = semget(key, 1, 0666 | IPC_CREAT);

    // initialize the mutex the first time the function is called
    // also initialize the semaphore which waits for the threads to exit
    if (num_threads == 0)
    {
        semctl(tmutex, 0, SETVAL, 1);
        semctl(tsem, 0, SETVAL, 0);

        // also add the parent thread to the table
        threads[num_threads].tid = gettid();
        threads[num_threads].stacksize = FOOTHREAD_DEFAULT_STACK_SIZE;
        threads[num_threads].jointype = FOOTHREAD_JOINABLE;
        threads[num_threads].ptid = gettid();
        threads[num_threads].child = 0;
        num_threads++;
    }
    
    // lock the mutex
    wait(tmutex);

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

    // unlock the mutex
    signal(tmutex);
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
    // lock the mutex
    key_t key = ftok("foothread.c", 1);
    int tmutex = semget(key, 1, 0666 | IPC_CREAT);
    key = ftok("foothread.c", 2);
    int tsem = semget(key, 1, 0666 | IPC_CREAT);

    wait(tmutex);

    // get the tid of the current thread
    int current_tid = gettid();

    // check if the thread is in the table
    for (int i = 0; i < num_threads; i++)
    {
        if (threads[i].tid == gettid() && threads[i].ptid != gettid())
        {
            // if the thread is joinable, signal tsem
            if (threads[i].jointype == FOOTHREAD_JOINABLE)
            {
                signal(tsem);
            }
            
            // clear the thread from the table
            threads[i].tid = 0;
            threads[i].stacksize = 0;
            threads[i].jointype = 0;
            threads[i].ptid = 0;
            threads[i].child = 0;

            // unlock the mutex
            signal(tmutex);
            return;
        }
        else if (threads[i].tid == gettid() && threads[i].ptid == gettid())
        {
            // it is the parent thread, wait for the children to finish
            signal(tmutex);
            while (threads[i].child > 0)
            {
                wait(tsem);
                threads[i].child--;
            }
        }
    }
}

// initialize the mutex
void foothread_mutex_init(foothread_mutex_t *mutex)
{
    key_t key = ftok("foothread.c", mutexkey++);
    mutex->mutid = semget(key, 1, 0666 | IPC_CREAT);
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
    key_t key = ftok("foothread.c", mutexkey++);
    barrier->mutid = semget(key, 1, 0666 | IPC_CREAT);
    semctl(barrier->mutid, 0, SETVAL, 1);
    key = ftok("foothread.c", barrierkey++);
    barrier->semid = semget(key, 1, 0666 | IPC_CREAT);
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
    wait(barrier->mutid);
    barrier->count--;
    signal(barrier->mutid);
}

// destroy the barrier
void foothread_barrier_destroy(foothread_barrier_t *barrier)
{
    // destroy the semaphores
    semctl(barrier->mutid, 0, IPC_RMID, 0);
    semctl(barrier->semid, 0, IPC_RMID, 0);
}
