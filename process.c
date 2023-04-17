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
        printf("7aga mesh shaghala fel process\n");
        exit(-1);
    }
    //TODO it needs to get the remaining time from somewhere
    remainingTime = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    printf("%d aho w fadely: %d w ma3aya quantum%d\n",getpid(),remainingTime,quantum);
    key_t pKey = ftok("key", 'i');
    int processmsgqid = msgget(pKey, 0666 | IPC_CREAT);
    int finished = 0;
    struct message msg;
    msg.mtype = 1001;
    printf("remaining time = %d",remainingTime);
    while (1)
    {
        int previousTime = getClk();
        printf("           remaining %d quantum: %d pid %d\n",remainingTime,quantum,getpid());
        if(remainingTime <= quantum)
        {
            printf("remaining %d quantum: %d pid %d\n",remainingTime,quantum,getpid());
            quantum = remainingTime;
            finished = 1;   
        }
        
        int count=quantum;
        while(count>0)
        {
            if(getClk()-previousTime == 1)
            {
                printf("    count: %d\n",count);
                printf("    current: %d\n",getClk());
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
            printf("process %d is terminated ",getpid());
            exit(0);
        } 
        else
        {
            msg.status = 0;
            // printf("ana 5alast 7ety aho 3and %d\n",getClk());
            if(!interrupt)
                msgsnd(processmsgqid, &msg, sizeof(struct message), 0);
            interrupt = 0;
            kill(getpid(),SIGSTOP);
        }
        sec=0;
        printf("%d\n",remainingTime);
    }
    
    destroyClk(false);
    
    return 0;
}

void handler(int signum){

    interrupt = 1;
    signal(signum,handler);
}