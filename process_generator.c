#include "headers.h"
#include <string.h>


void clearResources(int);

int algorithm=-1,quantum = 0;
pid_t pid;

void readFile()
{
    FILE* file;
    char line[100];
    
    file = fopen("processes.txt", "r");
    if (file == NULL) {
        printf("Failed to open file.");
        return ;
    }
    //#id arrival runtime priority
    int id,arrival,runtime,priority;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "#", 1) == 0) {
            continue; // Skip comment lines
        }
        if (sscanf(line, "%d\t%d\t%d\t%d", &id, &arrival, &runtime, &priority) == 4) {
            printf("%d %d %d %d\n", id, arrival, runtime, priority); //store instead
            // Process the integers as desired
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
    //int shmid = shmget(SHKEY, 4, IPC_CREAT | 0444);
    //printf("generator shmid: %d\n",shmid);
    

    pid = fork();

    if(pid == -1)
    {
        printf("Error in fork\n");
        exit(-1);
    }
    else if (pid == 0)
    {
        char alg_str[10];
        char qntm_str[10];
        sprintf(alg_str, "%d", algorithm);
        sprintf(qntm_str, "%d", quantum);
        printf("I will go to scheduler.out\n");
        execl("scheduler.out", alg_str, qntm_str, NULL);

    }
    printf("I won't go to scheduler.out\n");
}

int main(int argc, char * argv[])
{
    //signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    readFile();
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    chooseAlgorithm();
    // 3. Initiate and create the scheduler and clock processes.
    initiateChildren();
    // 4. Use this function after creating the clock process to initialize clock
    //initClk();
    // To get time use this
    //int x = getClk();
    //printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    int status;
    waitpid(pid, &status, 0);
    printf("Child process finished with status %d\n", status);
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
