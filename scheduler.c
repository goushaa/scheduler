#include "headers.h"
#include "queue.h" //for round robin

struct message {
    long mtype;
    int status;
};

int msgqid,sigshmid;
int * sigshmaddr;



struct PCB * processTable;
pid_t * pids;
int pidCounter = 0;
int algorithm,quantum;
Queue queue;



int remainingProcesses;

void handler1(int signo) {
    if(pidCounter == 0 && algorithm == 3)
    {
        init_queue(&queue);
    }
    struct process temp;
    int tempProcesses = *sigshmaddr;
    for(int i =0;i<tempProcesses;i++)
    {
        msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
        printf("recieve process,counter:%d, id: %d, arrival: %d, runtime: %d, priority: %d\n",pidCounter,temp.id,temp.arrival,temp.runtime,temp.priority);
        processTable[pidCounter].fileInfo.id = temp.id;
        processTable[pidCounter].fileInfo.arrival = temp.arrival;
        processTable[pidCounter].fileInfo.runtime = temp.runtime;
        processTable[pidCounter].fileInfo.priority = temp.priority;
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
        printf("counter %d has id: %d\n",pidCounter,pids[pidCounter]);
        processTable[pidCounter].pid = pids[pidCounter];
        enqueue(&queue, processTable[pidCounter]);
        pidCounter++;
    }
    
}

int main(int argc, char * argv[])
{
    signal(SIGUSR1, handler1);
    
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

    key_t pKey = ftok("key", 'i');
    int processmsgqid = msgget(pKey, 0666 | IPC_CREAT);
    struct message msg;

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
            
            struct PCB hamada;
            while(remainingProcesses>0)
            {
                while(is_empty(&queue))
                {

                }
                hamada = dequeue(&queue);
                printf("hamada info:\n");
                printf("fileInfo: id:%d arrival:%d runtime:%d priority:%d\n",hamada.fileInfo.id,hamada.fileInfo.arrival,hamada.fileInfo.runtime,hamada.fileInfo.priority);
                printf("pid: %d\n",hamada.pid);
                printf("Remaining Processes: %d, START:%d\n",remainingProcesses,getClk());
                kill(hamada.pid,SIGCONT);
                

                msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT);
                printf("ana 3adeet el recieve\n");
                if(msg.status == 1)
                {
                    printf("ID:%d finished at time: %d\n",hamada.pid,getClk());
                    remainingProcesses--;
                }
                else
                {
                    printf("ID:%d exited at time: %d\n",hamada.pid,getClk());
                    enqueue(&queue, hamada);
                }
               
            }
            msgctl(processmsgqid, IPC_RMID, NULL);
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
