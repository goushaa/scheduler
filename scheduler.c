#include "headers.h"
#include "queue.h"
#include "HPF.h"
#include "minHeap.c"

/***************************************************************Variables***************************************************************/
typedef struct buddyNode
{
    struct buddyNode *child[2];
    struct buddyNode *parent;
    int size;
    int id;
    int start;
    int smallest;
    bool full;
} node;

node Parent;

node **proccessNode;

// for shared memory
int msgqid, sigshmid;
int *sigshmaddr;

// for message queue
int processmsgqid;
struct message msg;

// for PCBs in the scheduler
struct PCB *processTable;
struct PCB *originProcessTable;
pid_t *pids;
int pidCounter = 0;

int processesNumber;
minHeap priorityQueue;

int remainingProcesses;

Queue queue; // for round robin
int algorithm, quantum;

// for the current PCB running
struct PCB *HPF_current_PCB;
struct PCB RR_current_PCB;

// allam
struct PCB *currentPCB;
int runningProcess = -1, prevTime = 0;
bool interrupt = 0;
void proccesEnd(struct PCB *process)
{
    process->end = getClk();
    process->waitingTime = process->start - process->fileInfo.arrival;
    process->turnaroundTime = process->end - process->fileInfo.arrival;
    process->executionTime = process->end - process->start;
}

// for memory
int memoryAlgorithm = -1;
int memory[1024];

/***************************************************************Memory Algorithms***************************************************************/
void First_Fit_Allocation(int id)
{
    processTable = originProcessTable;
    bool flag;
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (memory[i] == -1)
        {
            flag = true;
            for (int j = i; j < i + processTable[id].fileInfo.memSize; j++)
            {
                if (memory[j] != -1)
                {
                    flag = false;
                    break;
                }
            }

            if (flag)
            {
                for (int j = i; j < i + processTable[id].fileInfo.memSize; j++)
                    memory[j] = processTable[id].fileInfo.id;

                break;
            }
        }
    }

    if (i == 1024)
        printf("ERROR!!! Memory is Full\n");

    printf("Memory after Allocation:\n");
    for (int i = 0; i < 65; i++)
        printf("%d ", memory[i]);
    printf("\n\n");
}

void First_Fit_Deallocation(int id)
{
    processTable = originProcessTable;

    for (int i = 0; i < 1024; i++)
    {
        if (memory[i] == processTable[id].fileInfo.id)
        {
            for (int j = i; j < i + processTable[id].fileInfo.memSize; j++)
                memory[j] = -1;

            break;
        }
    }

    printf("Memory after Deallocation:\n");
    for (int i = 0; i < 65; i++)
        printf("%d ", memory[i]);
    printf("\n\n");
}

// 256
void printRange(node * current,bool flag){
    printf("%s",(flag?"Allocation ":"Deallocation "));
    printf("%d %d \n",current->start,current->start+current->size-1);
}

void setNode(node *current, node *parent, int id, int start, bool full)
{
    current->child[0] = current->child[1] = NULL;
    current->full = full;
    current->id = id;
    current->parent = parent;
    current->size = parent->size / 2;
    current->start = start;
    if (id != -1)
    {
        proccessNode[id] = current;
    }
}

void checkFullNode(node *current)
{
    if (current->child[0] != NULL && current->child[1] != NULL && current->child[0]->full && current->child[1]->full)
        current->full = 1;
}

node *findMIN(node *current, int size)
{
    node *ret = NULL;
    for (int i = 0; i < 2; i++)
    {
        if (current->child[i] && !current->child[i]->full)
        {
            node *temp = findMIN(current->child[i], size);
            if (temp != NULL && (ret == NULL || temp->size < ret->size))
                ret = temp;
        }
        else if (current->child[i] == NULL && current->size / 2 >= size && ret == NULL)
        {
            ret = current;
        }
    }

    return ret;
}

bool BUDDY_MEMORY_ALLOCATION(node *current, int id) // spliting function only
{
    processTable = originProcessTable;
    int size = processTable[id].fileInfo.memSize;
    if (size < current->size && size >= current->size / 2)
    {
        if (current->child[0] == NULL)
        {
            current->child[0] = malloc(sizeof(node));
            setNode(current->child[0], current, id, current->start, true);
            printRange(current->child[0],1);
        }
        else if (current->child[1] == NULL)
        {
            current->child[1] = malloc(sizeof(node));
            setNode(current->child[1], current, id, current->start + current->size / 2, true);
            printRange(current->child[1],1);
        }
        else
            return 0;

        checkFullNode(current);
        return 1;
    }
    for (int i = 0; i < 2; i++)
    {
        if (current->child[i] == NULL)
        {
            current->child[i] = malloc(sizeof(node));
            setNode(current->child[i], current, -1, current->start + i * current->size / 2, false);
        }
        if (!current->child[i]->full && BUDDY_MEMORY_ALLOCATION(current->child[i], id))
        {
            checkFullNode(current);
            return 1;
        }
    }

    return 0;
}

bool buddyMemoryAllocation(int id)
{
    node * mn = findMIN(&Parent, originProcessTable[id].fileInfo.memSize);
    if (mn == NULL)
        return 0;
    BUDDY_MEMORY_ALLOCATION(mn, id);
    return 1;
}

void BUDDY_MEMORY_DEALLOCATION(node *current,bool flag)
{
    if(flag)
        printRange(current,0);

    node *parent = current->parent;
    if (parent->child[0] == current)
    {
        free(parent->child[0]);
        parent->child[0] = NULL;
    }
    else
    {
        free(parent->child[1]);
        parent->child[1] = NULL;
    }
    while (true)
    {
        if (parent->parent != NULL && parent->child[0] == NULL && parent->child[1] == NULL)
        {
            BUDDY_MEMORY_DEALLOCATION(parent,0);
            break;
        }
        parent->full = false;
        if (parent->parent == NULL)
            break;
        parent = parent->parent;
    }
}

/***************************************************************Send/Recieve Handler***************************************************************/

// Handler for receiving signal from process generator when a process is arrived
void handler1(int signo)
{
    processTable = originProcessTable;

    if (pidCounter == 0)
    { // refactor later and make it initilazie in main
        if (algorithm == 2)
            initialize(processesNumber, &priorityQueue);
        if (algorithm == 3)
            init_queue(&queue);
    }

    if (algorithm == 2 && runningProcess != -1)
    {
        interrupt = 1;
        kill(runningProcess, SIGUSR1);
        int currentTime = getClk();
        if (prevTime < currentTime)
        {
            currentPCB->remainingTime--;
            currentPCB->heapPriority--;
        }
        if (currentPCB->remainingTime > 0)
            insertValue(currentPCB, &priorityQueue);
        else
        {
            remainingProcesses--;
            proccesEnd(currentPCB);
        }
    }

    struct process temp;
    int tempProcesses = *sigshmaddr;
    for (int i = 0; i < tempProcesses; i++)
    {
        msgrcv(msgqid, &temp, sizeof(struct process), 0, !IPC_NOWAIT);
        printf("Scheduler:: Recieve process-> id: %d, arrival: %d, runtime: %d, priority: %d, memSize: %d\n", temp.id, temp.arrival, temp.runtime, temp.priority, temp.memSize);
        processTable[pidCounter].fileInfo = temp;
        processTable[pidCounter].state = 0;
        processTable[pidCounter].remainingTime = temp.runtime;
        processTable[pidCounter].start = -1;

        switch (memoryAlgorithm)
        {
        case 1:
            First_Fit_Allocation(pidCounter);
            break;
        case 2:
            buddyMemoryAllocation(pidCounter);
            break;
        }

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
            insertValue(&processTable[pidCounter], &priorityQueue);
            break;
        case 3:
            enqueue(&queue, processTable[pidCounter]);
            break;
        }
        pidCounter++;
    }
}

/***************************************************************Scheduler Algorithms***************************************************************/
// Function for HPF implementation
void HPF_Algo()
{
    while (remainingProcesses > 0)
    {
        while (pisempty())
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
        switch (memoryAlgorithm)
        {
        case 1:
            First_Fit_Deallocation(HPF_current_PCB->fileInfo.id - 1);
            break;
        case 2:
            BUDDY_MEMORY_DEALLOCATION(proccessNode[HPF_current_PCB->fileInfo.id - 1],1);
            break;
        }
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
        if (RR_current_PCB.start == -1)
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
            switch (memoryAlgorithm)
            {
            case 1:
                First_Fit_Deallocation(RR_current_PCB.fileInfo.id - 1);
                break;
            case 2:
                break;
            }
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
    while (remainingProcesses > 0)
    {
        printf("enter and queue cnt = %d\n", getcount(&priorityQueue));
        while (isEmpty(&priorityQueue))
        {
        }
        currentPCB = heapExtractMin(&priorityQueue);
        runningProcess = currentPCB->pid;
        kill(currentPCB->pid, SIGCONT);
        printf("running procces %d and it's remaining time is %d\n", currentPCB->fileInfo.id, currentPCB->remainingTime);
        prevTime = getClk();
        if (currentPCB->start == -1)
            currentPCB->start = prevTime;

        currentPCB->state = 1;

        while (msgrcv(processmsgqid, &msg, sizeof(struct message), 1001, !IPC_NOWAIT) == -1 && !interrupt)
        {
        }
        printf("end\n");

        if (interrupt)
        {
            interrupt = 0;
            continue;
        }

        currentPCB->state = 0;

        if (msg.status == 1)
        {
            proccesEnd(currentPCB);
            printf("ID:%d finished at time: %d\n", currentPCB->pid, getClk());
            remainingProcesses--;
            switch (memoryAlgorithm)
            {
            case 1:
                First_Fit_Deallocation(currentPCB->fileInfo.id - 1);
                break;
            case 2:
                break;
            }
        }
        else
        {
            currentPCB->remainingTime--;
            currentPCB->heapPriority--;
            runningProcess = -1;
            insertValue(currentPCB, &priorityQueue);
        }
    }
}

/***************************************************************MAIN***************************************************************/
int main(int argc, char *argv[])
{
    signal(SIGUSR1, handler1);

    if (argc != 5)
    {
        printf("ERROR, few arguments\n");
    }

    algorithm = atoi(argv[1]);
    quantum = atoi(argv[2]);
    processesNumber = atoi(argv[3]);
    memoryAlgorithm = atoi(argv[4]);

    processTable = (struct PCB *)malloc(processesNumber * sizeof(struct PCB));
    proccessNode = malloc(processesNumber * sizeof(node *));
    memset(proccessNode, 0, sizeof(proccessNode)); // setting array of pointers to NULL

    originProcessTable = processTable;
    pids = (pid_t *)malloc(processesNumber * sizeof(pid_t));
    remainingProcesses = processesNumber;

    Parent.child[0] = Parent.child[1] = NULL;
    Parent.parent = NULL;
    Parent.id = -1;
    Parent.size = 1024;
    Parent.start = 0;
    Parent.smallest = 512;

    // initalize memory
    for (int i = 0; i < 1024; i++)
        memory[i] = -1;

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
    // destroyClk(true);
}
