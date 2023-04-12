#include "headers.h"
#include <string.h>

struct process {
    int id;
    int arrival; //IMPORTANT
    int runtime;
    int priority;
};

void clearResources(int);

int algorithm=-1,quantum = 0,processesNumber=0;
pid_t pids[2];

void readFile(struct process ** processes_ptr)
{
    FILE* file;
    char line[100];
    
    file = fopen("processes.txt", "r");
    if (file == NULL) {
        printf("Failed to open file.");
        return ;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "#", 1) == 0) {
            continue; // Skip comment lines
        }
        processesNumber++;
    }

    rewind(file);

    *processes_ptr = (struct process *) malloc(processesNumber * sizeof(struct process));

    printf("Rewinding with %d processes\n",processesNumber);

    //#id arrival runtime priority
    int id,arrival,runtime,priority;
    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "#", 1) == 0) {
            continue; // Skip comment lines
        }
        if (sscanf(line, "%d\t%d\t%d\t%d", &id, &arrival, &runtime, &priority) == 4) {
            (*processes_ptr+i)->id = id; // use *processes_ptr to access the allocated memory
            (*processes_ptr+i)->arrival = arrival;
            (*processes_ptr+i)->runtime = runtime;
            (*processes_ptr+i)->priority = priority;
            i++;
        }
    }
    
    fclose(file);
}

void chooseAlgorithm()
{
    while (algorithm != 1 && algorithm != 2 && algorithm != 3 )
    {
        printf("1. Non-preemptive Highest Priority First (HPF)\n");
        printf("2. Shortest Remaining time Next (SRTN)\n");
        printf("3. Round Robin (RR)\n");
        printf("Please enter the number of scheduling algorithm you want to execute: ");
        scanf("%d",&algorithm);
    }
    if(algorithm == 3)
    {
        
        while ( quantum < 1 )
        {
            printf("Please enter the quantum for the round robin algorithm: ");
            scanf("%d",&quantum);
        }
    }
    return;
    
}

void initiateChildren()
{
    for(int i=0;i<2;i++)
    {
        pids[i] = fork();
        if(pids[i] == -1)
        {
            printf("Error in fork\n");
            exit(-1);
        }
        else if (pids[i] == 0)
        {
            if (i == 0)
            {
                char alg_str[10];
                char qntm_str[10];
                sprintf(alg_str, "%d", algorithm);
                sprintf(qntm_str, "%d", quantum);
                execl("./scheduler.out","scheduler.out", alg_str, qntm_str, NULL);
            }
            else
                execl("./clk.out", "clk.out", NULL);

        }
    }
}

int main(int argc, char * argv[])
{
    //signal(SIGINT, clearResources);
    struct process * processes = NULL;
    // TODO Initialization
    // 1. Read the input files.
    readFile(&processes);
    for(int i=0;i<processesNumber;i++)
    {
        printf("when i = %d, id: %d, arrival: %d, runtime: %d, priority: %d\n",i,(processes+i)->id,(processes+i)->arrival,(processes+i)->runtime,(processes+i)->priority);
    }
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    chooseAlgorithm();
    // 3. Initiate and create the scheduler and clock processes.
    initiateChildren();
    
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    key_t key = ftok("key", 'p');
    int msgqid = msgget(key, 0666 | IPC_CREAT);
    int i = 0;
    while (i<processesNumber)
    {
        int currentTime = getClk();
        if((processes+i)->arrival == currentTime)
        {
            printf("current process, id: %d, arrival: %d, runtime: %d, priority: %d\n",(processes+i)->id,(processes+i)->arrival,(processes+i)->runtime,(processes+i)->priority);
            msgsnd(msgqid, &processes[i], sizeof(struct process), 0);
            i++;
        }
    }
    
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    int status;
    for(int i =0;i<2;i++)
    {
        waitpid(pids[i], &status, 0);
        printf("Child process finished with status %d\n", status);
    }
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
