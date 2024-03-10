#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "event.h"

// array of threads
pthread_t patients[50], reporters[20], salesreps[10];

// array of patient, reporter, and salesrep time and duration
int patient_time[50], reporter_time[20], salesrep_time[10];
int patient_duration[50], reporter_duration[20], salesrep_duration[10];

// global variables
int patient_count = 0, reporter_count = 0, salesrep_count = 0; // total number of patients, reporters, and salesreps
int patient_index = 0, reporter_index = 0, salesrep_index = 0; // current index of patient, reporter, and salesrep
int patient_wait = 0, reporter_wait = 0, salesrep_wait = 0;    // number of patients, reporters, and salesreps waiting

int done = 0;  // flag to indicate when doctor is done
int doc = 0;   // flag to indicate doctor is created
int t = 0;     // time
int dt = 0;    // time when doctor finishes current treatment
int sig = 0;   // signal the doctor
int treat = 0; // flag to indicate doctor is treating someone

// mutex and condition variables
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t smutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t pcond = PTHREAD_COND_INITIALIZER;
pthread_cond_t rcond = PTHREAD_COND_INITIALIZER;
pthread_cond_t scond = PTHREAD_COND_INITIALIZER;
pthread_cond_t dcond = PTHREAD_COND_INITIALIZER;

// function prototypes
void *patient(void *arg);
void *reporter(void *arg);
void *salesrep(void *arg);
void *doctor(void *arg);

int main()
{
    // main function, assistant thread

    // initialize event queue
    eventQ E = initEQ("arrival2.txt");

    // create doctor thread
    pthread_t doctor_thread;
    pthread_create(&doctor_thread, NULL, doctor, NULL);

    // insert coming of doctor event
    event doctor_coming;
    doctor_coming.time = 0;
    doctor_coming.type = 'D';
    doctor_coming.duration = 1;

    E = addevent(E, doctor_coming);

    // loop through the event queue
    while (!emptyQ(E))
    {
        // extract event
        event e = nextevent(E);
        E = delevent(E);

        // update time
        pthread_mutex_lock(&tmutex);
        t = e.time;
        int tt = t;
        pthread_mutex_unlock(&tmutex);

        // convert time to hours and minutes wrt 9 am
        tt += 540;
        int hours = tt / 60;
        int minutes = tt % 60;
        int am = 1;
        if (hours > 12)
        {
            am = 0;
            hours -= 12;
        }
        if (hours == 12)
            am = 0;

        // print event
        // printf("\t\t\t\tEvent Type: %c, Time: %02d:%02d %s\n", e.type, hours, minutes, am ? "AM" : "PM");

        // process event
        if (e.type == 'D')
        {
            // means doctor is available
            doc = 1;

            if (e.duration == 1)
            {
                pthread_mutex_lock(&dmutex);
                dt = 0;
                sig = 1;
                treat = 0;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);
            }

            // check if people are waiting
            // signal appropriate threads
            // priority: reporters > patients > salesreps

            pthread_mutex_lock(&rmutex);
            if (reporter_wait > 0)
            {
                // increment reporter index
                reporter_index++;
                reporter_wait--;
                pthread_cond_signal(&rcond);
                pthread_mutex_unlock(&rmutex);

                // insert next reporter event
                event next_reporter;
                next_reporter.time = t + reporter_duration[reporter_index];
                next_reporter.type = 'D';
                next_reporter.duration = 0;
                reporter_time[reporter_index] = t;

                E = addevent(E, next_reporter);

                // signal doctor
                pthread_mutex_lock(&dmutex);
                sig = 1;
                dt = reporter_time[reporter_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&rmutex);

            pthread_mutex_lock(&pmutex);
            if (patient_wait > 0)
            {
                // increment patient index
                patient_index++;
                patient_wait--;
                pthread_cond_signal(&pcond);
                pthread_mutex_unlock(&pmutex);

                // insert next patient event
                event next_patient;
                next_patient.time = t + patient_duration[patient_index];
                next_patient.type = 'D';
                next_patient.duration = 0;
                patient_time[patient_index] = t;

                E = addevent(E, next_patient);

                // signal doctor
                pthread_mutex_lock(&dmutex);
                sig = 1;
                dt = patient_time[patient_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&pmutex);

            pthread_mutex_lock(&smutex);
            if (salesrep_wait > 0)
            {
                // increment salesrep index
                salesrep_index++;
                salesrep_wait--;
                pthread_cond_signal(&scond);
                pthread_mutex_unlock(&smutex);

                // insert next salesrep event
                event next_salesrep;
                next_salesrep.time = t + salesrep_duration[salesrep_index];
                next_salesrep.type = 'D';
                next_salesrep.duration = 0;
                salesrep_time[salesrep_index] = t;

                E = addevent(E, next_salesrep);

                // signal doctor
                pthread_mutex_lock(&dmutex);
                sig = 1;
                dt = salesrep_time[salesrep_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&smutex);

            // if no one is waiting, no treatment
            pthread_mutex_lock(&dmutex);
            treat = 0;
            pthread_mutex_unlock(&dmutex);

            // check if quota is fulfilled for patients(25) and salesreps(3)
            pthread_mutex_lock(&dmutex);
            if ((patient_count > 25 && salesrep_count > 3))
            {
                // make done flag 1
                // signal doctor
                done = 1;
                sig = 1;
                dt = t;
                treat = 0;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);
            }
            pthread_mutex_unlock(&dmutex);
        }

        if (e.type == 'R')
        {
            // reporter arrived
            pthread_mutex_lock(&rmutex);
            reporter_count++;
            printf("\t\t[%02d:%02d %s] Reporter %d arrives\n", hours, minutes, am ? "AM" : "PM", reporter_count);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Reporter %d leaves (session over)\n", hours, minutes, am ? "AM" : "PM", reporter_count);
                pthread_mutex_unlock(&rmutex);
                continue;
            }

            // else create a thread, passing the reporter number
            pthread_create(&reporters[reporter_count], NULL, reporter, (void *)&reporter_count);

            // insert reporter duration in array
            reporter_duration[reporter_count] = e.duration;
            pthread_mutex_unlock(&rmutex);

            // check if doctor is not treating anyone
            pthread_mutex_lock(&dmutex);
            if (!treat && doc && !done)
            {
                // signal the reporter as well
                pthread_mutex_lock(&rmutex);
                reporter_index++;
                pthread_cond_signal(&rcond);
                pthread_mutex_unlock(&rmutex);

                // insert next reporter event
                event next_reporter;
                next_reporter.time = t + e.duration;
                next_reporter.type = 'D';
                next_reporter.duration = 0;
                reporter_time[reporter_index] = t;

                E = addevent(E, next_reporter);

                // signal doctor
                sig = 1;
                dt = reporter_time[reporter_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&dmutex);

            // else wait for doctor to be available
            pthread_mutex_lock(&rmutex);
            reporter_wait++;
            pthread_mutex_unlock(&rmutex);
        }

        if (e.type == 'P')
        {
            // patient arrived
            pthread_mutex_lock(&pmutex);
            patient_count++;
            printf("\t\t[%02d:%02d %s] Patient %d arrives\n", hours, minutes, am ? "AM" : "PM", patient_count);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Patient %d leaves (session over)\n", hours, minutes, am ? "AM" : "PM", patient_count);
                pthread_mutex_unlock(&pmutex);
                continue;
            }

            // check if already 25 patients have arrived
            if (patient_count > 25)
            {
                printf("\t\t[%02d:%02d %s] Patient %d leaves (quota full)\n", hours, minutes, am ? "AM" : "PM", patient_count);
                pthread_mutex_unlock(&pmutex);
                continue;
            }

            // else create a thread, passing the patient number
            pthread_create(&patients[patient_count], NULL, patient, (void *)&patient_count);

            // insert patient duration in array
            patient_duration[patient_count] = e.duration;
            pthread_mutex_unlock(&pmutex);

            // check if doctor is not treating anyone
            pthread_mutex_lock(&dmutex);
            if (!treat && doc && !done)
            {
                // signal the patient as well
                pthread_mutex_lock(&pmutex);
                patient_index++;
                pthread_cond_signal(&pcond);
                pthread_mutex_unlock(&pmutex);

                // insert next patient event
                event next_patient;
                next_patient.time = t + e.duration;
                next_patient.type = 'D';
                next_patient.duration = 0;
                patient_time[patient_index] = t;

                E = addevent(E, next_patient);

                // signal doctor
                sig = 1;
                dt = patient_time[patient_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&dmutex);

            // else wait for doctor to be available
            pthread_mutex_lock(&pmutex);
            patient_wait++;
            pthread_mutex_unlock(&pmutex);
        }

        if (e.type == 'S')
        {
            // salesrep arrived
            pthread_mutex_lock(&smutex);
            salesrep_count++;
            printf("\t\t[%02d:%02d %s] Salesrep %d arrives\n", hours, minutes, am ? "AM" : "PM", salesrep_count);

            // check if session is over
            if (done)
            {
                printf("\t\t[%02d:%02d %s] Salesrep %d leaves (quota full)\n", hours, minutes, am ? "AM" : "PM", salesrep_count);
                pthread_mutex_unlock(&smutex);
                continue;
            }

            // check if already 3 salesreps have arrived
            if (salesrep_count > 3)
            {
                printf("\t\t[%02d:%02d %s] Salesrep %d leaves (quota full)\n", hours, minutes, am ? "AM" : "PM", salesrep_count);
                pthread_mutex_unlock(&smutex);
                continue;
            }

            // else create a thread, passing the salesrep number
            pthread_create(&salesreps[salesrep_count], NULL, salesrep, (void *)&salesrep_count);

            // insert salesrep duration in array
            salesrep_duration[salesrep_count] = e.duration;
            pthread_mutex_unlock(&smutex);

            // check if doctor is not treating anyone
            pthread_mutex_lock(&dmutex);
            if (!treat && doc && !done)
            {
                // signal the salesrep as well
                pthread_mutex_lock(&smutex);
                salesrep_index++;
                pthread_cond_signal(&scond);
                pthread_mutex_unlock(&smutex);

                // insert next salesrep event
                event next_salesrep;
                next_salesrep.time = t + e.duration;
                next_salesrep.type = 'D';
                next_salesrep.duration = 0;
                salesrep_time[salesrep_index] = t;

                E = addevent(E, next_salesrep);

                // signal doctor
                sig = 1;
                dt = salesrep_time[salesrep_index];
                treat = 1;
                pthread_cond_signal(&dcond);
                pthread_mutex_unlock(&dmutex);

                continue;
            }
            pthread_mutex_unlock(&dmutex);

            // else wait for doctor to be available
            pthread_mutex_lock(&smutex);
            salesrep_wait++;
            pthread_mutex_unlock(&smutex);
        }

        usleep(100000);
    }

    // wait for doctor to finish
    // pthread_join(doctor_thread, NULL);

    return 0;
}

// doctor thread
void *doctor(void *arg)
{
    while (1)
    {
        // wait for signal
        pthread_mutex_lock(&dmutex);
        while (!sig)
            pthread_cond_wait(&dcond, &dmutex);

        // reset signal
        sig = 0;
        pthread_mutex_unlock(&dmutex);

        // extract current time
        pthread_mutex_lock(&tmutex);
        int tt = dt;
        pthread_mutex_unlock(&tmutex);

        // convert time to hours and minutes wrt 9 am
        tt += 540;
        int hours = tt / 60;
        int minutes = tt % 60;
        int am = 1;
        if (hours > 12)
        {
            am = 0;
            hours -= 12;
        }
        if (hours == 12)
            am = 0;

        // check if session is over
        if (done)
        {
            printf("[%02d:%02d %s] Doctor leaves (session over)\n", hours, minutes, am ? "AM" : "PM");
            pthread_exit(NULL);
        }

        // print that doctor has new visitor
        printf("[%02d:%02d %s] Doctor has new visitor\n", hours, minutes, am ? "AM" : "PM");
    }
}

// patient thread
void *patient(void *arg)
{
    int *p = (int *)arg;
    int patient_number = *p;

    // wait for doctor to be available
    pthread_mutex_lock(&pmutex);
    if (patient_index != patient_number)
        pthread_cond_wait(&pcond, &pmutex);
    pthread_mutex_unlock(&pmutex);

    // extract time and duration from array
    int tt = patient_time[patient_number];
    int duration = patient_duration[patient_number];

    // convert time to hours and minutes wrt 9 am
    tt += 540;
    int hours = tt / 60;
    int minutes = tt % 60;
    int am = 1;
    if (hours > 12)
    {
        am = 0;
        hours -= 12;
    }
    if (hours == 12)
        am = 0;

    int tt2 = tt + duration;
    int hours2 = tt2 / 60;
    int minutes2 = tt2 % 60;
    int am2 = 1;
    if (hours2 > 12)
    {
        am2 = 0;
        hours2 -= 12;
    }
    if (hours2 == 12)
        am2 = 0;

    // print that patient is being treated
    printf("[%02d:%02d %s - %02d:%02d %s] Patient %d is in doctor's chamber\n", hours, minutes, am ? "AM" : "PM", hours2, minutes2, am2 ? "AM" : "PM", patient_number);

    pthread_exit(NULL);
}

// reporter thread
void *reporter(void *arg)
{
    int *p = (int *)arg;
    int reporter_number = *p;

    // wait for doctor to be available
    pthread_mutex_lock(&rmutex);
    if (reporter_index != reporter_number)
        pthread_cond_wait(&rcond, &rmutex);
    pthread_mutex_unlock(&rmutex);

    // extract time and duration from array
    int tt = reporter_time[reporter_number];
    int duration = reporter_duration[reporter_number];

    // convert time to hours and minutes wrt 9 am
    tt += 540;
    int hours = tt / 60;
    int minutes = tt % 60;
    int am = 1;
    if (hours > 12)
    {
        am = 0;
        hours -= 12;
    }
    if (hours == 12)
        am = 0;

    int tt2 = tt + duration;
    int hours2 = tt2 / 60;
    int minutes2 = tt2 % 60;
    int am2 = 1;
    if (hours2 > 12)
    {
        am2 = 0;
        hours2 -= 12;
    }
    if (hours2 == 12)
        am2 = 0;

    // print that reporter is being treated
    printf("[%02d:%02d %s - %02d:%02d %s] Reporter %d is in doctor's chamber\n", hours, minutes, am ? "AM" : "PM", hours2, minutes2, am2 ? "AM" : "PM", reporter_number);

    pthread_exit(NULL);
}

// salesrep thread
void *salesrep(void *arg)
{
    int *p = (int *)arg;
    int salesrep_number = *p;

    // wait for doctor to be available
    pthread_mutex_lock(&smutex);
    if (salesrep_index != salesrep_number)
        pthread_cond_wait(&scond, &smutex);
    pthread_mutex_unlock(&smutex);

    // extract time and duration from array
    int tt = salesrep_time[salesrep_number];
    int duration = salesrep_duration[salesrep_number];

    // convert time to hours and minutes wrt 9 am
    tt += 540;
    int hours = tt / 60;
    int minutes = tt % 60;
    int am = 1;
    if (hours > 12)
    {
        am = 0;
        hours -= 12;
    }
    if (hours == 12)
        am = 0;

    int tt2 = tt + duration;
    int hours2 = tt2 / 60;
    int minutes2 = tt2 % 60;
    int am2 = 1;
    if (hours2 > 12)
    {
        am2 = 0;
        hours2 -= 12;
    }
    if (hours2 == 12)
        am2 = 0;

    // print that salesrep is being treated
    printf("[%02d:%02d %s - %02d:%02d %s] Salesrep %d is in doctor's chamber\n", hours, minutes, am ? "AM" : "PM", hours2, minutes2, am2 ? "AM" : "PM", salesrep_number);

    pthread_exit(NULL);
}