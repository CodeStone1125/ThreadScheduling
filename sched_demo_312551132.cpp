#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define MAXTHREAD 7
pthread_barrier_t barrier;
int num_thread;
double period_busy;

typedef struct {
    pthread_t thread_id;
    int printed_thread_id; // thread id to be printed out
    int sched_policy;
    int sched_priority;
} thread_info_t;

void *thread_func(void *arg);

void *thread_func(void *printed_thread_id) {
    // Awaiting the Readiness of All Threads
    pthread_barrier_wait(&barrier);
    
    // Executing the Task
    clock_t volatile wake_clock = clock() + period_busy * CLOCKS_PER_SEC;
    for (int i = 0; i < 3; i++) 
    {
        printf("Thread %d is running\n", *((int *)printed_thread_id));
        // Busy for <time_wait> seconds
        while (clock() < wake_clock);
        wake_clock += (clock_t)(period_busy * CLOCKS_PER_SEC);
    }
    
    // Exit the function
    return 0;
}

int main(int argc, char *argv[]) {
    thread_info_t thread_infos[MAXTHREAD];
    int opt, i;
    char *token;
    
    // Parsing Program Arguments
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
            case 'n':
                num_thread = atoi(optarg);
                break;
            case 't':
                period_busy = atof(optarg);
                break;
            case 's':
                i = 0;
                token = strtok(optarg, ",");
                while (token != NULL) {
                    thread_infos[i++].sched_policy = (strcmp(token, "NORMAL") == 0) ? SCHED_OTHER : SCHED_FIFO;
                    token = strtok(NULL, ",");
                }
                break;
            case 'p':
                i = 0;
                token = strtok(optarg, ",");
                while (token != NULL) {
                    thread_infos[i++].sched_priority = atoi(token);
                    token = strtok(NULL, ",");
                }
                break;
            default:
                break;
        }
    }

    // Create worker threads for a number of <num_threads>
    pthread_barrier_init(&barrier, NULL, num_thread + 1);
    for (int i = num_thread - 1; i >= 0; i--) 
    {
        thread_infos[i].printed_thread_id = i;
        pthread_create(&thread_infos[i].thread_id, NULL, thread_func, (void *)&thread_infos[i].printed_thread_id);
    }

    // Configuring CPU Affinity
    cpu_set_t cpuset_zero, cpuset_one;
    CPU_ZERO(&cpuset_zero);
    CPU_SET(0, &cpuset_zero);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset_zero);
    
    CPU_ZERO(&cpuset_one);
    CPU_SET(1, &cpuset_one);
    for (int i = 0; i < num_thread; i++) 
    {
        pthread_setaffinity_np(thread_infos[i].thread_id, sizeof(cpu_set_t), &cpuset_one); 
    }

    sched_param params[MAXTHREAD];
    for (int i = 0; i < num_thread; i++) 
    {
        // Assigning Attributes to Each Thread
        params[i].sched_priority = (thread_infos[i].sched_priority >= 0) ? thread_infos[i].sched_priority : 0;
        pthread_setschedparam(thread_infos[i].thread_id, thread_infos[i].sched_policy, &params[i]);        
    }

    // Simultaneously Initiating All Threads
    pthread_barrier_wait(&barrier);

    // Waiting for All Threads to Complete Execution
    for (int i = 0; i < num_thread; i++){
        pthread_join(thread_infos[i].thread_id, NULL);	
    }
}



