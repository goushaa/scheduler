#include "headers.h"

/* Modify this file as needed*/
int remainingTime;
bool interrupt = 0;
bool sec=0;
void handler(int signum);

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

 //   printf("remaining time = %d",remainingTime);

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
                sec=1;

            }   
            if(interrupt)break;  
        }

        if(sec && finished == 1)
        {
            msg.status = 1;
            msgsnd(processmsgqid, &msg, sizeof(struct message), 0);
            //printf("process %d is terminated ",getpid());
            exit(0);
        } 
        else
        {
            msg.status = 0;
            if(!interrupt)
                msgsnd(processmsgqid, &msg, sizeof(struct message), 0);
            interrupt = 0;
            kill(getpid(),SIGSTOP);
        }
        sec=0;
        //printf("%d\n",remainingTime);

    }
    
    //destroyClk(false);
    
    return 0;
}

void handler(int signum){

    interrupt = 1;
    signal(signum,handler);
}