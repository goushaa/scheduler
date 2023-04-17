#include "headers.h"
#include "queue.h" //for round robin
#include "HPF.h"
#include "minHeap.c"


// for shared memory
int msgqid, sigshmid;
int *sigshmaddr;

// for message queue
int processmsgqid;
struct message msg;

// for PCBs in the scheduler
struct PCB *processTable;
pid_t *pids;
int pidCounter = 0;

int processesNumber;
minHeap priorityQueue;

int remainingProcesses;

Queue queue; // for round robin
int algorithm, quantum;


//allam
struct PCB *currentPCB;
int runningProcess = -1,prevTime=0;
bool interrupt = 0;


void proccesEnd(struct PCB *process){
    process->end = getClk();
    process->waitingTime =process->start -process->fileInfo.arrival;
    process->turnaroundTime =process->end -process->fileInfo.arrival;
    process->executionTime =process->end -process->start;
}


// for the current PCB running
struct PCB *HPF_current_PCB;
struct PCB RR_current_PCB;

// Handler for receiving signal from process generator when a process is arrived

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
        printf("Scheduler:: Recieve process-> id: %d, arrival: %d, runtime: %d, priority: %d\n", temp.id, temp.arrival, temp.runtime, temp.priority);
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

// Function for HPF implementation
void HPF_Algo()
{
    while (remainingProcesses > 0)
    {
        while (pisempty() && HPF_current_PCB == NULL)
        {
        }

        /*Switch to the next PCB*/
        HPF_current_PCB = ppeek();
        pdequeue();
        kill(HPF_current_PCB->pid, SIGCONT);

        /*Set Start Time*/
        HPF_current_PCB->start = getClk();
        HPF_current_PCB->state = 1;

        /*Wait untill process finish its runtime*/
        while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1)
        {
        }

        /*Some Calculations*/
        HPF_current_PCB->end = getClk();
        HPF_current_PCB->waitingTime = HPF_current_PCB->start - HPF_current_PCB->fileInfo.arrival;
        HPF_current_PCB->turnaroundTime = HPF_current_PCB->end - HPF_current_PCB->fileInfo.arrival;
        HPF_current_PCB->executionTime = HPF_current_PCB->end - HPF_current_PCB->start;
        HPF_current_PCB->state = 0;
        remainingProcesses--;
        printf("Running process, id: %d, start: %d, finish: %d, priority: %d, ", HPF_current_PCB->fileInfo.id, HPF_current_PCB->start, HPF_current_PCB->end, HPF_current_PCB->fileInfo.priority);
        printf("waiting: %d, turnaround: %d, execution: %d\n\n", HPF_current_PCB->waitingTime, HPF_current_PCB->turnaroundTime, HPF_current_PCB->executionTime);
    }
}

// Function for RR implementation
void RR_Algo()
{
    while (remainingProcesses > 0)
    {
        while (is_empty(&queue))
        {
        }

        RR_current_PCB = dequeue(&queue);
        printf("Running process with id: %d running on time: %d\n", RR_current_PCB.fileInfo.id, getClk());
        kill(RR_current_PCB.pid, SIGCONT);
        RR_current_PCB.state = 1;
        // printf("Running process with id: %d return to run on time:%d",RR_current_PCB.fileInfo.id,getClk());

        /*Set Start Time*/
        if (RR_current_PCB.start == 0)
            RR_current_PCB.start = getClk();

        /*Wait untill process finish its quantum*/
        while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1)
        {
        }

        if (msg.status == 1)
        {
            /*Some Calculations*/
            RR_current_PCB.end = getClk();
            RR_current_PCB.turnaroundTime = RR_current_PCB.end - RR_current_PCB.fileInfo.arrival;
            RR_current_PCB.executionTime = RR_current_PCB.end - RR_current_PCB.start;
            RR_current_PCB.waitingTime = RR_current_PCB.turnaroundTime - RR_current_PCB.fileInfo.runtime;
            RR_current_PCB.state = 0;
            remainingProcesses--;
            printf("Running process, id: %d, start: %d, finish: %d, priority: %d, ", RR_current_PCB.fileInfo.id, RR_current_PCB.start, RR_current_PCB.end, RR_current_PCB.fileInfo.priority);
            printf("waiting: %d, turnaround: %d, execution: %d\n\n", RR_current_PCB.waitingTime, RR_current_PCB.turnaroundTime, RR_current_PCB.executionTime);
        }
        else
        {
            RR_current_PCB.state = 0;
            enqueue(&queue, RR_current_PCB);
        }
    }
}

// Function for SRTN implementation
void SRTN_Algo()
{
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
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler1);

    if (argc != 4)
    {
        printf("ERROR, few arguments\n");
    }

    algorithm = atoi(argv[1]);
    quantum = atoi(argv[2]);
    processesNumber = atoi(argv[3]);
    processTable = (struct PCB *)malloc(processesNumber * sizeof(struct PCB));
    pids = (pid_t *)malloc(processesNumber * sizeof(pid_t));
    remainingProcesses = processesNumber;

    // for message queue of receiving from process generator
    key_t key = ftok("key", 'p');
    msgqid = msgget(key, 0666 | IPC_CREAT);

    // shared memory of receiving from process generator for special case of coming many processes in same time
    key_t sigkey = ftok("key", 'n');
    sigshmid = shmget(sigkey, 4, 0666 | IPC_CREAT);
    sigshmaddr = (int *)shmat(sigshmid, (void *)0, 0);

    // for message queue of receiving from process
    key_t pKey = ftok("key", 'i');
    processmsgqid = msgget(pKey, 0666 | IPC_CREAT);

    initClk();

    switch (algorithm)
    {
    case 1:
        HPF_Algo();
        break;
    case 2:
       SRTN_Algo();
        break;
    case 3:
        RR_Algo();
        break;
    default:
        break;
    }
    msgctl(processmsgqid, IPC_RMID, NULL);
    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("\n........DONE........\n");
    //destroyClk(true);
}
