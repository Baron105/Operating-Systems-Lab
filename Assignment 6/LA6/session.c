#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "event.h"

pthread_mutex_t dmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dcond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pcond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t smutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t rcond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;

int num_patients = 0;
int num_sales = 0;
int num_reporters = 0;
int wait_patients = 0;
int wait_sales = 0;
int wait_reporters = 0;
int done = 0;
int t = 0;
int doc_created = 0;


void *doctor(void *arg)
{
    // doctor thread

    while (1)
    {
        // wait for the assistant to signal
        pthread_mutex_lock(&dmutex);

        // extract time and convert to 12 hr format wrt 9 am
        pthread_mutex_lock(&tmutex);
        int time = t + 9*60;
        pthread_mutex_unlock(&tmutex);
        int hr = time / 60;
        int min = time % 60;
        int am = 0;
        if (hr > 12)
        {
            hr -= 12;
            am = 1;
        }
        if (hr == 12)
        {
            am = 1;
        }

        // check if session is over
        if (done)
        {
            printf("[%02d:%02d%s] Doctor leaves (session over)\n", hr, min, am ? "pm" : "am");
            pthread_exit(NULL);
        }

        // print the doctor's arrival and exit time
        printf("[%02d:%02d%s] Doctor has new visitor\n", hr, min, am ? "pm" : "am");
    }
}

void *patient(void *arg)
{
    // patient thread

    // wait for the assistant to signal

    // get the patient number, arrival time and duration
    int *p = (int *)arg;
    int num = p[0];
    pthread_mutex_lock(&tmutex);
    int arrival = t;
    pthread_mutex_unlock(&tmutex);
    int duration = p[2];

    // calculate the arrival time and exit time in 12 hr format
    int arrival_hr = (arrival + 9*60) / 60;
    int arrival_min = (arrival + 9*60) % 60;
    int arrival_am = 0;
    if (arrival_hr > 12)
    {
        arrival_hr -= 12;
        arrival_am = 1;
    }
    if (arrival_hr == 12)
    {
        arrival_am = 1;
    }

    int exit_hr = (arrival + 9*60 + duration) / 60;
    int exit_min = (arrival + 9*60 + duration) % 60;
    int exit_am = 0;
    if (exit_hr > 12)
    {
        exit_hr -= 12;
        exit_am = 1;
    }
    if (exit_hr == 12)
    {
        exit_am = 1;
    }

    

    // print the arrival and exit time
    printf("[%02d:%02d%s - %02d:%02d%s] Patient %d is in doctor's chamber\n", arrival_hr, arrival_min, arrival_am ? "pm" : "am", exit_hr, exit_min, exit_am ? "pm" : "am", num);

    pthread_mutex_lock(&pmutex);

    pthread_exit(NULL);
}

void *sales(void *arg)
{
    // sales representative thread

    // wait for the assistant to signal

    // get the sales number, arrival time and duration
    int *p = (int *)arg;
    int num = p[0];
    pthread_mutex_lock(&tmutex);
    int arrival = t;
    pthread_mutex_unlock(&tmutex);
    int duration = p[2];

    // calculate the arrival time and exit time in 12 hr format
    int arrival_hr = (arrival + 9*60) / 60;
    int arrival_min = (arrival + 9*60) % 60;
    int arrival_am = 0;
    if (arrival_hr > 12)
    {
        arrival_hr -= 12;
        arrival_am = 1;
    }
    if (arrival_hr == 12)
    {
        arrival_am = 1;
    }

    int exit_hr = (arrival + 9*60 + duration) / 60;
    int exit_min = (arrival + 9*60 + duration) % 60;
    int exit_am = 0;
    if (exit_hr > 12)
    {
        exit_hr -= 12;
        exit_am = 1;
    }
    if (exit_hr == 12)
    {
        exit_am = 1;
    }

    

    // print the arrival and exit time
    printf("[%02d:%02d%s - %02d:%02d%s] Sales representative %d is in doctor's chamber\n", arrival_hr, arrival_min, arrival_am ? "pm" : "am", exit_hr, exit_min, exit_am ? "pm" : "am", num);

    pthread_mutex_lock(&smutex);

    pthread_exit(NULL);
}

void *reporter(void *arg)
{
    // reporter thread

    // wait for the assistant to signal

    // get the reporter number, arrival time and duration
    int *p = (int *)arg;
    int num = p[0];
    pthread_mutex_lock(&tmutex);
    int arrival = t;
    pthread_mutex_unlock(&tmutex);
    int duration = p[2];

    // calculate the arrival time and exit time in 12 hr format
    int arrival_hr = (arrival + 9*60) / 60;
    int arrival_min = (arrival + 9*60) % 60;
    int arrival_am = 0;
    if (arrival_hr > 12)
    {
        arrival_hr -= 12;
        arrival_am = 1;
    }
    if (arrival_hr == 12)
    {
        arrival_am = 1;
    }

    int exit_hr = (arrival + 9*60 + duration) / 60;
    int exit_min = (arrival + 9*60 + duration) % 60;
    int exit_am = 0;
    if (exit_hr > 12)
    {
        exit_hr -= 12;
        exit_am = 1;
    }
    if (exit_hr == 12)
    {
        exit_am = 1;
    }

    

    // print the arrival and exit time
    printf("[%02d:%02d%s - %02d:%02d%s] Reporter %d is in doctor's chamber\n", arrival_hr, arrival_min, arrival_am ? "pm" : "am", exit_hr, exit_min, exit_am ? "pm" : "am", num);

    pthread_mutex_lock(&rmutex);

    pthread_exit(NULL);
}

int main()
{
    // assistant thread

    // read arrival.txt and call initEQ function to create a min heap
    eventQ E = initEQ("arrival2.txt");

    // insert doctor event into the min heap
    event doctor_event;
    doctor_event.type = 'D';
    doctor_event.time = 0;
    doctor_event.duration = 1;

    pthread_mutex_lock(&rmutex);
    pthread_mutex_lock(&pmutex);
    pthread_mutex_lock(&smutex);
    pthread_mutex_lock(&dmutex);


    E = addevent(E, doctor_event);

    // create a thread for the doctor
    pthread_t tid;
    pthread_create(&tid, NULL, doctor, NULL);

    // go through the min heap and print the events
    while (!emptyQ(E))
    {
        event e = nextevent(E);

        // remove the event from the min heap
        E = delevent(E);

        // extract time and convert to 12 hr format wrt 9 am
        int time = e.time + 9*60;
        int hr = time / 60;
        int min = time % 60;
        int am = 0;
        if (hr > 12)
        {
            hr -= 12;
            am = 1;
        }
        if (hr == 12)
        {
            am = 1;
        }


            // update the time
        pthread_mutex_lock(&tmutex);
        t = e.time;
        pthread_mutex_unlock(&tmutex);
        
        // check event type
        // if it is R or P or S
        if (e.type == 'R')
        {
            num_reporters++;
            printf("\t\t[%02d:%02d%s] Reporter %d arrives\n", hr, min, am ? "pm" : "am", num_reporters);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d%s] Reporter %d leaves (session over)\n", hr, min, am ? "pm" : "am", num_reporters);
                continue;
            }

            // create a thread for the reporter
            pthread_t tid;
            int *p = (int *)malloc(3 * sizeof(int));
            p[0] = num_reporters;
            p[1] = e.time;
            p[2] = e.duration;
            pthread_create(&tid, NULL, reporter, (void *)p);

            // insert the reporter exit event into the min heap
            event reporter_exit;
            reporter_exit.type = 'D';
            reporter_exit.time = e.time + e.duration;
            reporter_exit.duration = 0;

            E = addevent(E, reporter_exit);

            wait_reporters++;
        }

        if (e.type == 'P')
        {
            num_patients++;
            printf("\t\t[%02d:%02d%s] Patient %d arrives\n", hr, min, am ? "pm" : "am", num_patients);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d%s] Patient %d leaves (session over)\n", hr, min, am ? "pm" : "am", num_patients);
                continue;
            }

            // check if quota is full
            else if (num_patients > 25)
            {
                printf("\t\t[%02d:%02d%s] Patient %d leaves (quota full)\n", hr, min, am ? "pm" : "am", num_patients);
                continue;
            }

            // create a thread for the patient
            pthread_t tid;
            int *p = (int *)malloc(3 * sizeof(int));
            p[0] = num_patients;
            p[1] = e.time;
            p[2] = e.duration;
            pthread_create(&tid, NULL, patient, (void *)p);

            // insert the patient exit event into the min heap
            event patient_exit;
            patient_exit.type = 'D';
            patient_exit.time = e.time + e.duration;
            patient_exit.duration = 0;

            E = addevent(E, patient_exit);

            wait_patients++;
        }

        if (e.type == 'S')
        {
            num_sales++;
            printf("\t\t[%02d:%02d%s] Sales representative %d arrives\n", hr, min, am ? "pm" : "am", num_sales);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d%s] Sales representative %d leaves (session over)\n", hr, min, am ? "pm" : "am", num_sales);
                continue;
            }

            // check if quota is full
            else if (num_sales > 3)
            {
                printf("\t\t[%02d:%02d%s] Sales representative %d leaves (quota full)\n", hr, min, am ? "pm" : "am", num_sales);
                continue;
            }

            // create a thread for the sales representative
            pthread_t tid;
            int *p = (int *)malloc(3 * sizeof(int));
            p[0] = num_sales;
            p[1] = e.time;
            p[2] = e.duration;
            pthread_create(&tid, NULL, sales, (void *)p);

            // insert the sales representative exit event into the min heap
            event sales_exit;
            sales_exit.type = 'D';
            sales_exit.time = e.time + e.duration;
            sales_exit.duration = 0;

            E = addevent(E, sales_exit);

            wait_sales++;
        }

        if (e.type == 'D')
        {
            // signal the doctor thread if the duration is 1, ie the initial doctor event
            if (e.duration == 1)
            {
                doc_created = 1;
            }

            // signal the doctor thread
            if (doc_created)
            {
                pthread_mutex_unlock(&dmutex);
            }

            // signal the appropriate thread
            // priority order reporter > patient > sales representative

            if (wait_reporters > 0 && doc_created)
            {
                wait_reporters--;
                pthread_mutex_unlock(&rmutex);
            }
            else if (wait_patients > 0 && doc_created)
            {
                wait_patients--;
                pthread_mutex_unlock(&pmutex);
            }
            else if (wait_sales > 0 && doc_created)
            {
                wait_sales--;
                pthread_mutex_unlock(&smutex);
            }
        }

        if (num_patients >= 25 && num_sales >= 3)
        {
            done = 1;
        }
    }
    printf("\n");
}

