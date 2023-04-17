#include "headers.h"
#include "queue.h" //for round robin
#include "HPF.h"
#include "minHeap.c"

int msgqid, sigshmid;
int *sigshmaddr;

struct PCB *processTable;
pid_t *pids;
int pidCounter = 0;
int algorithm, quantum;
int processesNumber;
Queue queue;
minHeap priorityQueue;


int remainingProcesses;

// Mustafa
struct PCB *current_PCB;

//allam
struct PCB *currentPCB;
int runningProcess = -1,prevTime=0;
bool interrupt = 0;

struct PCB hamada;
bool hamed = false;

void proccesEnd(struct PCB *process){
    process->end = getClk();
    process->waitingTime =process->start -process->fileInfo.arrival;
    process->turnaroundTime =process->end -process->fileInfo.arrival;
    process->executionTime =process->end -process->start;
}

void handler1(int signo)
{
    if(pidCounter == 0 )
    {   // refactor later and make it initilazie in main
        if(algorithm==2)
        initialize(processesNumber,&priorityQueue);
        if(algorithm==3)
        init_queue(&queue);
        
    }
    
    if(algorithm == 2 && runningProcess != -1){
        interrupt = 1;
        kill(runningProcess,SIGUSR1);
        int currentTime=getClk();
        if(prevTime<currentTime){
            currentPCB->remainingTime--;
            currentPCB->heapPriority--;
        }
        if(currentPCB->remainingTime > 0)
            insertValue(currentPCB,&priorityQueue);
        else {
            remainingProcesses--;
            proccesEnd(currentPCB);
        }
    }

    struct process temp;
    int tempProcesses = *sigshmaddr;
    for (int i = 0; i < tempProcesses; i++)
    {
        msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
        printf("recieve process,counter:%d, id: %d, arrival: %d, runtime: %d, priority: %d\n", pidCounter, temp.id, temp.arrival, temp.runtime, temp.priority);
        processTable[pidCounter].fileInfo = temp;
        processTable[pidCounter].state = 0;
        processTable[pidCounter].remainingTime = temp.runtime;
        processTable[pidCounter].start = -1;

        pids[pidCounter] = fork();
        if (pids[pidCounter] == 0)
        {
            char runtime_str[10]; // assuming the maximum number of digits for runtime is 10
            char quantum_str[10]; // assuming the maximum number of digits for runtime is 10
            sprintf(runtime_str, "%d", temp.runtime);
            if (algorithm == 1)
                quantum = temp.runtime;
            sprintf(quantum_str, "%d", quantum);
            execl("./process.out", "process.out", runtime_str, quantum_str, NULL);
        }
        else if (pids[pidCounter] == -1)
        {
            printf("error in fork\n");
        }
        kill(pids[pidCounter], SIGSTOP); // stop all the processes
        processTable[pidCounter].pid = pids[pidCounter];
        switch (algorithm)
        {
        case 1:
            penqueue(&processTable[pidCounter], temp.priority);
            break;
        case 2:
            processTable[pidCounter].heapPriority = processTable[pidCounter].remainingTime;
            insertValue(&processTable[pidCounter],&priorityQueue);
            break;
        case 3:
            enqueue(&queue, processTable[pidCounter]);
            break;
        }
        pidCounter++;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler1);

    if (argc != 4)
    {
        printf("sad ya5oya\n");
    }

    algorithm = atoi(argv[1]);
    quantum = atoi(argv[2]);
    processesNumber = atoi(argv[3]);
    processTable = (struct PCB *)malloc(processesNumber * sizeof(struct PCB));
    pids = (pid_t *)malloc(processesNumber * sizeof(pid_t));
    remainingProcesses = processesNumber;

    printf("%d aho w ma3aya, ", getpid());
    printf("algorithm: %d, ", algorithm);
    printf("quantum: %d, ", quantum);
    printf("no.: %d\n", processesNumber);

    key_t key = ftok("key", 'p');
    msgqid = msgget(key, 0666 | IPC_CREAT);

    key_t pKey = ftok("key", 'i');
    int processmsgqid = msgget(pKey, 0666 | IPC_CREAT);
    struct message msg;

    key_t sigkey = ftok("key", 'n');
    sigshmid = shmget(sigkey, 4, 0666 | IPC_CREAT);
    sigshmaddr = (int *)shmat(sigshmid, (void *)0, 0);
    printf("sig hehe: %d\n", sigshmid);
    initClk();
    switch (algorithm)
    {
    case 1:
        while (remainingProcesses > 0)
        {
            while (pisempty() && current_PCB == NULL)
            {
            }

            /*Switch to the next PCB*/
            current_PCB = ppeek();
            pdequeue();
            kill(current_PCB->pid, SIGCONT);
            /*Set Start Time*/
            current_PCB->start = getClk();
            current_PCB->state = 1;

            while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1)
            {
            }
            /*Some Calculations*/
            current_PCB->end = getClk();
            current_PCB->waitingTime = current_PCB->start - current_PCB->fileInfo.arrival;
            current_PCB->turnaroundTime = current_PCB->end - current_PCB->fileInfo.arrival;
            current_PCB->executionTime = current_PCB->end - current_PCB->start;
            current_PCB->state = 0;
            printf("running process, id: %d, start: %d, finish: %d, priority: %d\n", current_PCB->fileInfo.id, current_PCB->start, current_PCB->end, current_PCB->fileInfo.priority);

            remainingProcesses--;
        }
        break;
    case 2:
        while(remainingProcesses>0)
            {
                printf("enter and queue cnt = %d\n",getcount(&priorityQueue));
                while(isEmpty(&priorityQueue)){

                }
                currentPCB = heapExtractMin(&priorityQueue);
                runningProcess = currentPCB->pid;
                kill(currentPCB->pid,SIGCONT);
                printf("running procces %d and it's remaining time is %d\n",currentPCB->fileInfo.id,currentPCB->remainingTime);
                prevTime=getClk();
                if(currentPCB->start == -1 ) 
                    currentPCB->start = prevTime;
                
                currentPCB->state = 1;

                while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1 && !interrupt)
                {

                }
                printf("end\n");

                if(interrupt){
                    interrupt = 0;
                    continue;
                }
                
                currentPCB->state = 0;

                if(msg.status == 1){
                    proccesEnd(currentPCB);
                    printf("ID:%d finished at time: %d\n", currentPCB->pid, getClk());
                    remainingProcesses--;
                }
                else{
                    currentPCB->remainingTime--;
                    currentPCB->heapPriority--;
                    runningProcess = -1;
                    insertValue(currentPCB,&priorityQueue);
                }

            }
        break;
    case 3:
        while (remainingProcesses > 0)
        {
            while (is_empty(&queue))
            {
            }

            hamada = dequeue(&queue);
            printf("hamada info:\n");
            printf("fileInfo: id:%d arrival:%d runtime:%d priority:%d\n", hamada.fileInfo.id, hamada.fileInfo.arrival, hamada.fileInfo.runtime, hamada.fileInfo.priority);
            printf("pid: %d\n", hamada.pid);
            printf("Remaining Processes: %d, START:%d\n", remainingProcesses, getClk());
            kill(hamada.pid, SIGCONT);
            printf("xxxxxxxxx\n");

            while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1)
            {
                printf("YYYYYYYYYY\n");
            }

            printf("ana 3adeet el recieve\n");
            if (msg.status == 1)
            {
                printf("ID:%d finished at time: %d\n", hamada.pid, getClk());
                remainingProcesses--;
            }
            else
            {
                printf("ID:%d exited at time: %d\n", hamada.pid, getClk());
                enqueue(&queue, hamada);
            }
        }
        break;
    default:
        printf("Invalid value\n");
        break;
    }
    msgctl(processmsgqid, IPC_RMID, NULL);

    printf("\nDONE\n");

    // TODO implement the scheduler :)
    // upon termination release the clock resources.

    destroyClk(true);
}
