#include "headers.h"

int msgqid,sigshmid;
int * sigshmaddr;

struct process {
    int id;
    int arrival; //IMPORTANT
    int runtime;
    int priority;
};

int remainingProcesses;

void handler(int signo) {
    struct process temp;
    int tempProcesses = *sigshmaddr;
    for(int i =0;i<tempProcesses;i++)
    {
        msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
        printf("recieve process, id: %d, arrival: %d, runtime: %d, priority: %d\n",temp.id,temp.arrival,temp.runtime,temp.priority);
    }
    
    remainingProcesses -= tempProcesses;
}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, handler);
    
    if(argc != 4)
    {
        printf("sad ya5oya\n");
    }
    
    int algorithm = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    int processesNumber = atoi(argv[3]);
    remainingProcesses = processesNumber;
    printf("%d aho w ma3aya, ",getpid());
    printf("algorithm: %d, ",algorithm);
    printf("quantum: %d, ",quantum);
    printf("no.: %d\n",processesNumber);

    key_t key = ftok("key", 'p');
    msgqid = msgget(key, 0666 | IPC_CREAT);

    key_t sigkey = ftok("key", 'n');
    sigshmid = shmget(sigkey, 4, 0666| IPC_CREAT);
    sigshmaddr = (int *) shmat(sigshmid, (void *)0, 0);
    printf("sig hehe: %d\n",sigshmid);
    initClk();
    while(remainingProcesses>0)
    {
        
    }
    printf("\nDONE\n");

    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
