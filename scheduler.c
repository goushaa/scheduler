#include "headers.h"

struct process {
    int id;
    int arrival; //IMPORTANT
    int runtime;
    int priority;
};

int main(int argc, char * argv[])
{
    initClk();
    if(argc != 3)
    {
        printf("sad ya5oya\n");
    }
    
    int algorithm = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    printf("ana aho w ma3aya, ");
    printf("algorithm: %d, ",algorithm);
    printf("quantum: %d\n",quantum);

    key_t key = ftok("key", 'p');
    int msgqid = msgget(key, 0666 | IPC_CREAT);

    while(1)
    {
        struct process temp;
        msgrcv(msgqid, &temp, sizeof(struct process), 0, 0);
        sleep(1);
        printf("recieve process, id: %d, arrival: %d, runtime: %d, priority: %d\n",temp.id,temp.arrival,temp.runtime,temp.priority);
    }

    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    //destroyClk(true);
}
