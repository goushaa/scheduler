#include "headers.h"
#include "queue.h" //for round robin

int msgqid,sigshmid;
int * sigshmaddr;

struct PCB * processTable;
pid_t * pids;
int pidCounter = 0;
int algorithm,quantum;
Queue queue;

int remainingProcesses;

int main(int argc, char * argv[])
{
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

    key_t newkey = ftok("key", 'q');
    int newshmid = shmget(newkey, 4, 0);
    if (newshmid == -1)
    {
        printf("Error in getting newshmid\n");
    }
    else
        printf("\nShared memory ID = %d\n", newshmid);

    int * newshmaddr = (int *) shmat(newshmid, (void *)0, 0);
    if (*newshmaddr == -1)
    {
        printf("Error in attach in newshmid\n");
    }

    key_t key = ftok("key", 'p');
    msgqid = msgget(key, 0666 | IPC_CREAT);

    key_t pKey = ftok("key", 's');
    int remainingshmid = shmget(pKey,4, 0666 | IPC_CREAT);

    if (remainingshmid == -1)
        printf("Error in create\n");
    else
        printf("\nShared memory ID = %d\n", remainingshmid);

    int *remshmaddr = (int *)shmat(remainingshmid, (void *)0, 0);
    if (remshmaddr == (void *)-1)
    {
        printf("Error in attach in server\n");
    }

    struct sembuf rem_sem_op;
    key_t sKey = ftok("key", 'd');
    int semid = semget(sKey, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        printf("Error in schedular semid create\n");
    }
    if (semctl(semid, 0, SETVAL, 0) == -1)
    {
        printf("Error in semctl\n");
    }
    
    key_t sigkey = ftok("key", 'n');
    sigshmid = shmget(sigkey, 4, 0666| IPC_CREAT);
    sigshmaddr = (int *) shmat(sigshmid, (void *)0, 0);
    printf("sig hehe: %d\n",sigshmid);
    if(pidCounter == 0 && algorithm == 3)
    {
        init_queue(&queue);
    }
    initClk();
    while (remainingProcesses>0)
    {
        //check new
        printf("new process hena: %d\n",*newshmaddr);
        while(is_empty(&queue) && *newshmaddr==0)
        {
        }
        
        if(*newshmaddr == 1)
        {
            struct process temp;
            int tempProcesses = *sigshmaddr;
            for(int i =0;i<tempProcesses;i++)
            {
                msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
                printf("recieve process,id: %d, arrival: %d, runtime: %d, priority: %d\n",temp.id,temp.arrival,temp.runtime,temp.priority);
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
                processTable[pidCounter].pid = pids[pidCounter];
                enqueue(&queue, processTable[pidCounter]);
                pidCounter++;
                *newshmaddr = 0;
            }
        }
        
        //perform algorithm
        switch (algorithm) {
            case 1:
                break;
            case 2:
                break;
            case 3:
                
                struct PCB hamada;
                hamada = dequeue(&queue);
                printf("TIME:%d process: id:%d arrival:%d runtime:%d priority:%d pid:%d\n",getClk(),hamada.fileInfo.id,hamada.fileInfo.arrival,hamada.fileInfo.runtime,hamada.fileInfo.priority,hamada.pid);
                kill(hamada.pid,SIGCONT);
                
                //down//
                rem_sem_op.sem_num = 0;
                rem_sem_op.sem_op = -1;
                rem_sem_op.sem_flg = !IPC_NOWAIT;
                semop(semid, &rem_sem_op, 1);
                //down//

                int remainingTime = *remshmaddr;
                if(remainingTime == 0)
                {
                    printf("process with id:%d finished at TIME: %d with remainingTime %d\n",hamada.pid,getClk(),remainingTime);
                    remainingProcesses--;
                }
                else
                {
                    printf("process with id:%d exited at TIME: %d with remainingTime %d\n",hamada.pid,getClk(),remainingTime);
                    enqueue(&queue, hamada);
                }  
                if(remainingProcesses== 0)
                {
                    semctl(semid,0, IPC_RMID);
                    shmctl(remainingshmid, IPC_RMID, (struct shmid_ds *)0);
                }
                break;
            default:
                printf("Invalid value\n");
                break;
        }
    }
    
    
    printf("\nDONE\n");

    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
