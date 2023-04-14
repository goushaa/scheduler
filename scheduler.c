#include "headers.h"
#include "queue.h" //for round robin

int msgqid,sigshmid;
int * sigshmaddr;



struct PCB * processTable;
pid_t * pids;
int pidCounter = 0;
int algorithm,quantum;
Queue queue;
int flag;


int remainingProcesses;

void handler1(int signo) {
    struct process temp;
    int tempProcesses = *sigshmaddr;
    for(int i =0;i<tempProcesses;i++)
    {
        msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
        printf("recieve process,counter:%d, id: %d, arrival: %d, runtime: %d, priority: %d\n",pidCounter,temp.id,temp.arrival,temp.runtime,temp.priority);
        if(algorithm == 3)
        {
            processTable[pidCounter].fileInfo = temp;
            processTable[pidCounter].state = 0;
            if(is_empty(&queue))
                processTable[pidCounter].state = 1;
            enqueue(&queue, processTable[pidCounter]);
            
        }
        pids[pidCounter] = fork();
        if(pids[pidCounter] == 0)
        {
            char runtime_str[10]; // assuming the maximum number of digits for runtime is 10
            char quantum_str[10]; // assuming the maximum number of digits for runtime is 10
            sprintf(runtime_str, "%d", temp.runtime);
            sprintf(quantum_str, "%d", quantum);
            execl("./process.out","process.out", runtime_str,quantum_str, NULL);
            
        } else if (pids[pidCounter] == -1)
        {
            printf("error in fork\n");
        }
        kill(pids[pidCounter],SIGSTOP);
        pidCounter++;
    }
    
}

void handler2(int signo) {
   flag = 1;
}

void handler3(int signo) {
   flag = 0;
}

int main(int argc, char * argv[])
{
    flag = -1;
    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);
    signal(SIGINT, handler3);
    
    if(argc != 4)
    {
        printf("sad ya5oya\n");
    }
    
    algorithm = atoi(argv[1]);
    quantum = atoi(argv[2]);
    int processesNumber = atoi(argv[3]);
    processTable = (struct PCB *) malloc(processesNumber  * sizeof(struct PCB));
    pids = (pid_t *) malloc(processesNumber * sizeof(pid_t));
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
    switch (algorithm) {
        case 1:
            while(remainingProcesses>0)
            {
                
            }
            break;
        case 2:
            while(remainingProcesses>0)
            {
                
            }
            break;
        case 3:
            init_queue(&queue);
            struct PCB hamada;
            while(remainingProcesses>0)
            {
                while(is_empty(&queue))
                {

                }
                hamada = dequeue(&queue);
                printf("Remaining Processes: %d, ID:%d, START:%d\n",remainingProcesses,hamada.fileInfo.id,getClk());
                kill(hamada.pid,SIGCONT);
                pause();
                if(flag == 1)
                {
                    printf("ID:%d exited at time: %d\n",hamada.fileInfo.id,getClk());
                    enqueue(&queue, hamada);
                }
                else if (flag == 0)
                {
                    printf("ID:%d finished at time: %d\n",hamada.fileInfo.id,getClk());
                    remainingProcesses--;
                }
                else
                {
                    printf("feh error\n");
                }
                flag = -1;

            }
            break;
        default:
            printf("Invalid value\n");
            break;
    }
    
    printf("\nDONE\n");

    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
