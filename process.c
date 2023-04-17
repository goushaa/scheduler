#include "headers.h"

/* Modify this file as needed*/
int remainingTime;


int main(int agrc, char * argv[])
{
    initClk();
    if(agrc != 3)
    {
        exit(-1);
    }
    //TODO it needs to get the remaining time from somewhere
    remainingTime = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    
    key_t pKey = ftok("key", 'i');
    int processmsgqid = msgget(pKey, 0666 | IPC_CREAT);

    int finished = 0;
    struct message msg;
    msg.mtype = 1001;

    while (1)
    {
        //printf("Process %d: having RemainingTime = %d and Quantum = %d\n",getpid(),remainingTime,quantum);
        int previousTime = getClk();
        if(remainingTime <= quantum)
        {
            quantum = remainingTime;
            finished = 1;   
        }
        
        int count=quantum;
        while(count>0)
        {
            if(getClk()-previousTime == 1)
            {
                //printf("Count: %d\n",count);
                //printf("Current: %d\n",getClk());
                count--;
                remainingTime--;
                previousTime = getClk();
            }   
        }

        if(finished == 1)
        {
            msg.status = 1;
            msgsnd(processmsgqid, &msg, sizeof(struct message), 0);
            exit(0);
        } 
        else
        {
            msg.status = 0;
            msgsnd(processmsgqid, &msg, sizeof(struct message), 0);
            kill(getpid(),SIGSTOP);
        }
    }
    
    //destroyClk(false);
    
    return 0;
}
