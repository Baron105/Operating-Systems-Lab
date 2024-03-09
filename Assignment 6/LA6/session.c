#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "event.h"

// create array of threads for patients, reporters, and sales reps
pthread_t patients[50], reporters[20], sales[10];

// another array to store duration of each patient, reporter, and sales rep
int patient_duration[50], reporter_duration[20], sale_duration[10];
int patient_time[50], reporter_time[20], sale_time[10];

// global variables
int num_patients = 0, num_reporters = 0, num_sales = 0;
int wait_patients = 0, wait_reporters = 0, wait_sales = 0;
int curr_patients = 0, curr_reporters = 0, curr_sales = 0;
int done = 0;
int doc = 0;
int timet = 0;
int sig = 0;
int treat = 0;

// mutexes
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t smutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trmutex = PTHREAD_MUTEX_INITIALIZER;

// barriers
pthread_barrier_t pbarrier;
pthread_barrier_t rbarrier;
pthread_barrier_t sbarrier;

// condition variables
pthread_cond_t pcond = PTHREAD_COND_INITIALIZER;
pthread_cond_t rcond = PTHREAD_COND_INITIALIZER;
pthread_cond_t scond = PTHREAD_COND_INITIALIZER;
pthread_cond_t dcond = PTHREAD_COND_INITIALIZER;

// function prototypes
void *patient(void *arg);
void *reporter(void *arg);
void *sale(void *arg);
void *doctor(void *arg);

// main function, assistant thread
int main()
{
    // initialise event queue
    eventQ E = initEQ("arrival2.txt");

    // lock all mutexes

    // insert coming of doctor to thread
    event doc_create;
    doc_create.time = 0;
    doc_create.type = 'D';
    doc_create.duration = 1;

    E = addevent(E, doc_create);

    pthread_t doctor_thread;
    pthread_create(&doctor_thread, NULL, doctor, NULL);

    // go through the event queue
    while (!emptyQ(E))
    {
        event e = nextevent(E);
        E = delevent(E);

        // update time
        pthread_mutex_lock(&tmutex);
        timet = e.time;
        int timett = timet + 9*60;
        pthread_mutex_unlock(&tmutex);

        // convert time to 12 hour format wrt 9 AM
        int hr = timett / 60;
        int min = timett % 60;
        int am = 1;
        if (hr > 12)
        {
            am = 0;
            hr -= 12;
        }
        if (hr == 12)
        {
            am = 0;
        }

        // printf("\t\t\t\tEvent type: %c, Time: %02d:%02d %s\n", e.type, hr, min, am ? "AM" : "PM");

        // check event type and create thread accordingly
        if (e.type == 'D')
        {
            pthread_mutex_lock(&dmutex);
            doc = 1;
            sig = 1;
            pthread_mutex_unlock(&dmutex);
            pthread_cond_signal(&dcond);
            pthread_mutex_unlock(&dmutex);

            // signal appropriate threads
            // priority: reporters > patients > sales

            pthread_mutex_lock(&rmutex);
            if (wait_reporters > 0 && curr_reporters < num_reporters)
            {
                curr_reporters++;
                wait_reporters--;

                // insert next reporter event
                // use duration of reporter to calculate next event
                event next_reporter;
                next_reporter.time = timet + reporter_duration[curr_reporters];
                next_reporter.type = 'D';
                next_reporter.duration = 0;
                reporter_time[num_reporters] = timet;
                E = addevent(E, next_reporter);
                pthread_mutex_unlock(&rmutex);
                pthread_cond_signal(&rcond);
                continue;
            }
            pthread_mutex_unlock(&rmutex);

            pthread_mutex_lock(&pmutex);
            if (wait_patients > 0 && curr_patients < num_patients)
            {
                curr_patients++;
                wait_patients--;

                // insert next patient event
                // use duration of patient to calculate next event
                event next_patient;
                next_patient.time = timet + patient_duration[curr_patients];
                next_patient.type = 'D';
                next_patient.duration = 0;
                patient_time[num_patients] = timet;
                E = addevent(E, next_patient);
                pthread_mutex_unlock(&pmutex);
                pthread_cond_signal(&pcond);
                continue;
            }
            pthread_mutex_unlock(&pmutex);

            pthread_mutex_lock(&smutex);
            if (wait_sales > 0 && curr_sales < num_sales)
            {
                curr_sales++;
                wait_sales--;

                // insert next sales event
                // use duration of sales rep to calculate next event
                event next_sales;
                next_sales.time = timet + sale_duration[curr_sales];
                next_sales.type = 'D';
                next_sales.duration = 0;
                sale_time[num_sales] = timet;
                E = addevent(E, next_sales);
                pthread_mutex_unlock(&smutex);
                pthread_cond_signal(&scond);
                continue;
            }
            pthread_mutex_unlock(&smutex);

            pthread_mutex_lock(&trmutex);
            treat = 0;
            pthread_mutex_unlock(&trmutex);
        }

        if (e.type == 'R')
        {
            pthread_mutex_lock(&rmutex);
            num_reporters++;
            printf("\t\t[%02d:%02d %s] Reporter %d arrives\n", hr, min, am ? "AM" : "PM", num_reporters);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Reporter %d leaves (session over)\n", hr, min, am ? "AM" : "PM", num_reporters);
                pthread_mutex_unlock(&rmutex);
                continue;
            }

            // pass relevant information to reporter thread
            int *arg = (int *)malloc(3 * sizeof(int));
            arg[0] = num_reporters;
            arg[1] = e.time;
            arg[2] = e.duration;

            // create reporter thread
            pthread_create(&reporters[num_reporters], NULL, reporter, (void *)arg);

            // insert duration of reporter
            reporter_duration[num_reporters] = e.duration;

            wait_reporters++;

            // check if the doctor is available and no one is in the chamber
            pthread_mutex_lock(&trmutex);
            if (doc == 1 && !done && !treat)
            {
                curr_reporters++;
                wait_reporters--;

                // insert next reporter event
                // use duration of reporter to calculate next event
                event next_reporter;
                next_reporter.time = timet + reporter_duration[num_reporters];
                next_reporter.type = 'D';
                next_reporter.duration = 0;
                reporter_time[num_reporters] = timet;
                E = addevent(E, next_reporter);
                pthread_mutex_unlock(&trmutex);
                pthread_mutex_unlock(&rmutex);
                pthread_cond_signal(&rcond);
                continue;
            }
            pthread_mutex_unlock(&trmutex);
            pthread_mutex_unlock(&rmutex);
        }

        if (e.type == 'P')
        {
            pthread_mutex_lock(&pmutex);
            num_patients++;
            printf("\t\t[%02d:%02d %s] Patient %d arrives\n", hr, min, am ? "AM" : "PM", num_patients);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Patient %d leaves (session over)\n", hr, min, am ? "AM" : "PM", num_patients);
                pthread_mutex_unlock(&pmutex);
                continue;
            }

            // check if quota is full
            else if (num_patients > 25)
            {
                printf("\t\t[%02d:%02d %s] Patient %d leaves (quota full)\n", hr, min, am ? "AM" : "PM", num_patients);
                pthread_mutex_unlock(&pmutex);
                continue;
            }

            // pass relevant information to patient thread
            int *arg = (int *)malloc(3 * sizeof(int));
            arg[0] = num_patients;
            arg[1] = e.time;
            arg[2] = e.duration;

            // insert duration of patient
            patient_duration[num_patients] = e.duration;

            // create patient thread
            pthread_create(&patients[num_patients], NULL, patient, (void *)arg);

            wait_patients++;

            // check if the doctor is available and no one is in the chamber
            pthread_mutex_lock(&trmutex);
            if (doc == 1 && !done && !treat)
            {
                curr_patients++;
                wait_patients--;

                // insert next patient event
                // use duration of patient to calculate next event
                event next_patient;
                next_patient.time = timet + patient_duration[num_patients];
                next_patient.type = 'D';
                next_patient.duration = 0;
                patient_time[num_patients] = timet;
                E = addevent(E, next_patient);
                pthread_mutex_unlock(&trmutex);
                pthread_mutex_unlock(&pmutex);
                pthread_cond_signal(&pcond);
                continue;
            }
            pthread_mutex_unlock(&trmutex);
            pthread_mutex_unlock(&pmutex);

        }

        if (e.type == 'S')
        {
            pthread_mutex_lock(&smutex);
            num_sales++;
            printf("\t\t[%02d:%02d %s] Sales representative %d arrives\n", hr, min, am ? "AM" : "PM", num_sales);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Sales representative %d leaves (session over)\n", hr, min, am ? "AM" : "PM", num_sales);
                pthread_mutex_unlock(&smutex);
                continue;
            }

            // check if quota is full
            else if (num_sales > 3)
            {
                printf("\t\t[%02d:%02d %s] Sales representative %d leaves (quota full)\n", hr, min, am ? "AM" : "PM", num_sales);
                pthread_mutex_unlock(&smutex);
                continue;
            }

            // pass relevant information to sales thread
            int *arg = (int *)malloc(3 * sizeof(int));
            arg[0] = num_sales;
            arg[1] = e.time;
            arg[2] = e.duration;

            // create sales thread
            pthread_create(&sales[num_sales], NULL, sale, (void *)arg);

            // insert duration of sales rep
            sale_duration[num_sales] = e.duration;

            wait_sales++;

            // check if the doctor is available and no one is in the chamber
            pthread_mutex_lock(&trmutex);
            if (doc == 1 && !done && !treat)
            {
                curr_sales++;
                wait_sales--;

                // insert next sales event
                // use duration of sales rep to calculate next event
                event next_sales;
                next_sales.time = timet + sale_duration[num_sales];
                next_sales.type = 'D';
                next_sales.duration = 0;
                sale_time[num_sales] = timet;
                E = addevent(E, next_sales);
                pthread_mutex_unlock(&trmutex);
                pthread_mutex_unlock(&smutex);
                pthread_cond_signal(&scond);
                continue;
            }
            pthread_mutex_unlock(&trmutex);
            pthread_mutex_unlock(&smutex);
        }
        pthread_mutex_lock(&dmutex);
        if ((num_patients > 25 && num_sales == 3) || (num_patients == 25 && num_sales > 3))
        {
            done = 1;
            sig = 1;
        }
        pthread_mutex_unlock(&dmutex);
        pthread_cond_signal(&dcond);

        usleep(50);

    }
}

// doctor thread
void *doctor(void *arg)
{
    while (1)
    {
        // wait for assistant to signal
        pthread_mutex_lock(&dmutex);

        while (sig == 0)
        {
            pthread_cond_wait(&dcond, &dmutex);
        }
        pthread_mutex_lock(&trmutex);
        treat = 1;
        pthread_mutex_unlock(&trmutex);

        sig = 0;

        // extract time
        pthread_mutex_lock(&tmutex);
        int t = timet + 9*60;
        pthread_mutex_unlock(&tmutex);

        // convert time to 12 hour format wrt 9 AM
        int hr = t / 60;
        int min = t % 60;
        int am = 1;
        if (hr > 12)
        {
            am = 0;
            hr -= 12;
        }
        if (hr == 12)
        {
            am = 0;
        }

        // check if session is over
        if (done)
        {
            printf("[%02d:%02d %s] Doctor leaves (session over)\n", hr, min, am ? "AM" : "PM");
            pthread_mutex_unlock(&dmutex);
            pthread_exit(NULL);
        }

        // print that doctor is available
        printf("[%02d:%02d %s] Doctor has new visitor\n", hr, min, am ? "AM" : "PM");
        pthread_mutex_unlock(&dmutex);
    }
}

// patient thread
void *patient(void *arg)
{
    // extract relevant information
    int *info = (int *)arg;
    int token = info[0];
    int d = info[2];


    // wait for your turn under condition variable
    pthread_mutex_lock(&pmutex);
    while (curr_patients != token)
    {
        pthread_cond_wait(&pcond, &pmutex);
    }

    // get the time
    int curr_time = patient_time[token] + 9*60;

    // convert time to 12 hour format wrt 9 AM
    int hr = curr_time / 60;
    int min = curr_time % 60;
    int am = 1;
    if (hr > 12)
    {
        am = 0;
        hr -= 12;
    }
    if (hr == 12)
    {
        am = 0;
    }

    int exit_time = curr_time + d;
    int exit_hr = exit_time / 60;
    int exit_min = exit_time % 60;
    int exit_am = 1;
    if (exit_hr > 12)
    {
        exit_am = 0;
        exit_hr -= 12;
    }
    if (exit_hr == 12)
    {
        exit_am = 0;
    }

    // print that patient is being attended
    printf("[%02d:%02d %s - %02d:%02d %s] Patient %d is in doctor's chamber\n", hr, min, am ? "AM" : "PM", exit_hr, exit_min, exit_am ? "AM" : "PM", token);

    // update current patients
    pthread_mutex_unlock(&pmutex);

    pthread_exit(NULL);
}

// reporter thread
void *reporter(void *arg)
{
    // extract relevant information
    int *info = (int *)arg;
    int token = info[0];
    int d = info[2];

    // wait for your turn under condition variable
    pthread_mutex_lock(&rmutex);
    while (curr_reporters != token)
    {
        pthread_cond_wait(&rcond, &rmutex);
    }
    // get the time
    int curr_time = reporter_time[token] + 9*60;

    // convert time to 12 hour format wrt 9 AM
    int hr = curr_time / 60;
    int min = curr_time % 60;
    int am = 1;
    if (hr > 12)
    {
        am = 0;
        hr -= 12;
    }
    if (hr == 12)
    {
        am = 0;
    }

    int exit_time = curr_time + d;
    int exit_hr = exit_time / 60;
    int exit_min = exit_time % 60;
    int exit_am = 1;
    if (exit_hr > 12)
    {
        exit_am = 0;
        exit_hr -= 12;
    }
    if (exit_hr == 12)
    {
        exit_am = 0;
    }

    // print that reporter is being attended
    printf("[%02d:%02d %s - %02d:%02d %s] Reporter %d is in doctor's chamber\n", hr, min, am ? "AM" : "PM", exit_hr, exit_min, exit_am ? "AM" : "PM", token);

    // update current reporters
    pthread_mutex_unlock(&rmutex);

    pthread_exit(NULL);
}

// sales thread
void *sale(void *arg)
{
    // extract relevant information
    int *info = (int *)arg;
    int token = info[0];
    int d = info[2];

    // wait for your turn under condition variable
    pthread_mutex_lock(&smutex);
    while (curr_sales != token)
    {
        pthread_cond_wait(&scond, &smutex);
    }

    // get the time
    int curr_time = sale_time[token] + 9*60;

    // convert time to 12 hour format wrt 9 AM
    int hr = curr_time / 60;
    int min = curr_time % 60;
    int am = 1;
    if (hr > 12)
    {
        am = 0;
        hr -= 12;
    }
    if (hr == 12)
    {
        am = 0;
    }

    int exit_time = curr_time + d;
    int exit_hr = exit_time / 60;
    int exit_min = exit_time % 60;
    int exit_am = 1;
    if (exit_hr > 12)
    {
        exit_am = 0;
        exit_hr -= 12;
    }
    if (exit_hr == 12)
    {
        exit_am = 0;
    }

    // print that sales rep is being attended
    printf("[%02d:%02d %s - %02d:%02d %s] Sales representative %d is in doctor's chamber\n", hr, min, am ? "AM" : "PM", exit_hr, exit_min, exit_am ? "AM" : "PM", token);

    // update current sales reps
    pthread_mutex_unlock(&smutex);

    pthread_exit(NULL);
}
