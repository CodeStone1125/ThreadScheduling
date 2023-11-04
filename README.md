Scheduling Policy Demonstration Program
===

Homework2 for NYCU Operation System aut 2023

* [Assignment 2: Scheduling Policy Demonstration Program](https://hackmd.io/@Bmch4MS0Rz-VZWB74huCvw/rJ8OLx6fp)

### How I implemented the program 
> Describe how you implemented the program in detail.


The main thread's implementation can be broken down into six distinct stages:

1. Parsing Program Arguments

    Utilize `getopt` to interpret command line options and `strtok` to split and identify each thread's scheduling policy and priority.
    ```cpp
    thread_info_t thread_infos[MAXTHREAD];
    int opt, i;
    char *token;
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            num_thread = atoi(optarg);
            break;

        case 't':
            period_busy = atof(optarg);
            break;

        case 's':
            i = 0;
            token = strtok(optarg, ",");
            while (token != NULL)
            {
                thread_infos[i++].sched_policy = (strcmp(token, "NORMAL") == 0) ? SCHED_OTHER : SCHED_FIFO;
                token = strtok(NULL, ",");
            }
            break;

        case 'p':
            i = 0;
            token = strtok(optarg, ",");
            while (token != NULL)
            {
                thread_infos[i++].sched_priority = atoi(token);
                token = strtok(NULL, ",");
            }
            break;
        default:
            break;
        }
    }
    ```
2. Create worker threads for number of <num_threads>

    * Initialize a barrier using `pthread_barrier_init` to immediately block the threads after their creation.
    * Employ `pthread_create` to generate a specified number of threads with default attributes.
    ```cpp
    pthread_barrier_init(&barrier, NULL, num_thread + 1);
    for (int i = 0; i < num_thread; i++)
    {
        thread_infos[i].printed_thread_id = i;
        pthread_create(&thread_infos[i].thread_id, NULL, thread_func, (void *) &thread_infos[i].printed_thread_id);
    }
    ```

    
3. Configuring CPU Affinity

    * Assign CPU affinity to the main thread, setting it to CPU core zero.
    * Also, apply CPU affinity to the other threads, all set to CPU core zero, using `pthread_setaffinity_np`.
    ```cpp
    cpu_set_t cpuset_zero, cpuset_one;
    CPU_ZERO(&cpuset_zero);
    CPU_SET(0, &cpuset_zero);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset_zero);
    
    CPU_ZERO(&cpuset_one);
    CPU_SET(1, &cpuset_one);
    for (int i = 0; i < num_thread; i++) {
        pthread_setaffinity_np(thread_infos[i].thread_id, sizeof(cpu_set_t), &cpuset_one); 
    }

    ```
4. Assigning Attributes to Each Thread

    * Utilize `pthread_setschedparam` to establish scheduling policies and priorities for each thread.
    * Note the distinction between the functions `pthread_setschedparam` and `pthread_attr_setschedparam`.
    ```cpp
    sched_param params[MAXTHREAD];
    for (int i = 0; i < num_thread; i++) {
        params[i].sched_priority = (thread_infos[i].sched_priority >= 0) ? thread_infos[i].sched_priority : 0;
        pthread_setschedparam(thread_infos[i].thread_id, thread_infos[i].sched_policy, &params[i]);        
    }    
    ```
5. Simultaneously Initiating All Threads

    * Initially, the created threads are held back by the barrier. Reinitiate all threads at this stage since their attributes have been previously modified.
    ```cpp
    pthread_barrier_wait(&barrier);
    ```
6. Waiting for All Threads to Complete Execution

    * Wait until all the threads created have finished executing the `thread_func`
    ```cpp
    for (int i = 0; i < num_thread; i++)
        pthread_join(thread_infos[i].thread_id, NULL);
    ```

The thread function, `thread_func`, can be segmented into two primary steps:
1. Awaiting the Readiness of All Threads

    Explicitly block the thread until all other threads have been generated and configured.
    ```cpp
    pthread_barrier_wait(&barrier);
    ```
2. Executing the Task

    Display the required text and engage in busy-waiting for a specified period, repeating this process three times.
    ```cpp
    clock_t volatile wake_clock = clock() + period_busy * CLOCKS_PER_SEC;
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", *((int *) printed_thread_id));
        /* Busy for <time_wait> seconds */
        while (clock() < wake_clock);
        wake_clock += (clock_t) (period_busy * CLOCKS_PER_SEC);
    }
    ```
    
### Test Result
Finally, here is the test result:
 ```bash
sudo ./sched_test.sh ./sched_demo ./sched_demo_312551132
[sudo] 312551132 的密碼： 
Running testcase 1: ./sched_demo -n 1 -t 0.5 -s NORMAL -p -1 ......
Result: Success!
Running testcase 2: ./sched_demo -n 2 -t 0.5 -s FIFO,FIFO -p 10,20 ......
Result: Success!
Running testcase 3: ./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30 ......
Result: Success!
 ```
### Cases Discussion
> Describe the results of `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30` and what causes that.

The result of command `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30` is shown below.
```
Thread 2 is running
Thread 2 is running
Thread 2 is running
Thread 1 is running
Thread 1 is running
Thread 1 is running
Thread 0 is running
Thread 0 is running
Thread 0 is running
```
Three threads have been defined. Threads 1 and 2 are configured as real-time tasks using the FIFO scheduling policy. Given that Thread 2 holds a higher priority compared to Thread 1, it is executed before Thread 1. Thread 0 is held in abeyance until all real-time tasks are completed.

> Describe the results of `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30` and what causes that.

The result of command `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30` is shown below.
```
Thread 3 is running
Thread 3 is running
Thread 3 is running
Thread 1 is running
Thread 1 is running
Thread 1 is running
Thread 2 is running
Thread 0 is running
Thread 2 is running
Thread 0 is running
Thread 2 is running
Thread 0 is running
```
Four threads have been designated for execution. Threads 1 and 3 are configured as real-time tasks, utilizing the FIFO scheduling policy. Given that Thread 3 holds a higher priority compared to Thread 1, it is executed before Thread 1. Threads 0 and 2 are held in abeyance until all real-time tasks conclude their execution. They operate under the SCHED_OTHER policy, commonly known as the Completely Fair Scheduler (CFS). As a result, Threads 0 and 2 take turns executing for a specified duration before being preempted.

### N-second-busy-waiting
> Describe how did you implement n-second-busy-waiting?

The n-second busy-waiting is achieved by continuously invoking the `clock()` function to monitor the passage of time. This approach ensures that the process consistently compares the current time against our designated waiting time, preventing it from entering a sleep state induced by the kernel.

```cpp
while (clock() < wake_clock);
    wake_clock += (clock_t) (period_busy * CLOCKS_PER_SEC);
```
