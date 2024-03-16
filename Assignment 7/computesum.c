// program to compute the sum of nodes in a tree
#include "foothread.h"

// number of nodes in the tree
int n;

// global arrays to store the parent, number of children and sum of each node
int P[100];
int C[100] = {0};
int S[100] = {0};

// synchronization variables, one barrier for each node and a mutex
foothread_barrier_t barrier[100];
foothread_mutex_t mutex;

// function to compute the sum of the nodes
int compute_sum(void *arg)
{
    // get the id of the thread after locking the mutex
    foothread_mutex_lock(&mutex);
    int *ptid = (int *)arg;
    int id = *ptid;

    // thread info
    // printf("thread: %02d tid: %d pid: %d\n", id, gettid(), getpid());

    // check if the node is a leaf
    if (C[id] == 0)
    {
        // it is a leaf node, ask the user for the value
        printf("Leaf node %3d :: Enter a positive integer: ", id);
        scanf("%d", &S[id]);

        // release mutex and wait for the other threads to finish
        foothread_mutex_unlock(&mutex);
        // actually, not needed as the max for any leaf node's barrier is 1, will unlock immediately
        // but kept for consistency
        foothread_barrier_wait(&barrier[id]);
    }
    else
    {
        // it is not a leaf node
        // wait for the children to finish, then compute the sum
        foothread_mutex_unlock(&mutex);
        // barrier will be released when all children have finished
        foothread_barrier_wait(&barrier[id]);

        // lock the mutex to update the sum
        foothread_mutex_lock(&mutex);
        for (int i = 0; i < n; i++)
        {
            if (P[i] == id && i != id) // check if the node is a child of the current node, and not the same as the current node
            {
                S[id] += S[i];
            }
        }

        printf("Internal node %3d gets the partial sum %d from its children", id, S[id]);

        // print the children, not needed in assignment, good for debugging
        for (int i = 0; i < n; i++)
        {
            if (P[i] == id && i != id)
            {
                printf(" %d", i);
            }
        }

        printf("\n");
        // release the mutex
        foothread_mutex_unlock(&mutex);
    }

    // wait for the parent to finish, every thread has to do this
    foothread_barrier_wait(&barrier[P[id]]);

    // signals the leader thread that the thread has finished
    foothread_exit();

    return 0;
}

int main()
{
    // read tree.txt file
    FILE *fp = fopen("tree.txt", "r");

    // check if the file exists
    if (fp == NULL)
    {
        perror("File opening failed\n");
        exit(1);
    }

    // read the number of nodes
    fscanf(fp, "%d", &n);

    // read the parent of each node
    for (int i = 0; i < n; i++)
    {
        int node, parent;
        fscanf(fp, "%d %d", &node, &parent);
        P[node] = parent;
        C[parent]++;
        // printf("%d %d\n", node, parent);

        if (node == parent)
            C[parent]--; // root node has itself as parent, so decrement the count by 1
    }

    // close the file, not needed anymore
    fclose(fp);

    // synchronization variables
    // initialize the mutex
    foothread_mutex_init(&mutex);

    // initialize the barrier for each node
    for (int i = 0; i < n; i++)
    {
        foothread_barrier_init(&barrier[i], C[i] + 1);
    }

    // create n threads
    for (int i = 0; i < n; i++)
    {
        // make a thread for each node
        // initialize the attr with the default values
        foothread_t thread;
        foothread_attr_t attr = FOOTHREAD_ATTR_INITIALIZER;

        // set the jointype to joinable, as default is detached
        // also set the stacksize to 1MB
        foothread_attr_setjointype(&attr, FOOTHREAD_JOINABLE);
        foothread_attr_setstacksize(&attr, 1024 * 1024);

        // pass the id of the node as the argument
        int *arg = (int *)malloc(sizeof(int));
        *arg = i;

        foothread_create(&thread, &attr, compute_sum, (void *)arg);

        // printf("Thread %d created\n", i);
    }

    // wait for the threads to finish
    foothread_exit();

    // find the node who's parent is itself
    int root;
    for (int i = 0; i < n; i++)
    {
        if (P[i] == i)
        {
            root = i;
            break;
        }
    }

    printf("Sum at root (node %d) = %d\n", root, S[root]);

    // destroy the synchronization variables
    foothread_mutex_destroy(&mutex);
    for (int i = 0; i < n; i++)
    {
        foothread_barrier_destroy(&barrier[i]);
    }

    return 0;
}