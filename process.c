#include "headers.h"

/* Modify this file as needed*/
int remainingTime;

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
    int finished = 0;
    while (remainingTime > 0)
    {
        int previousTime = getClk();
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
                count--;
                remainingTime--;
                previousTime = getClk();
            }   
        }
        if(finished == 1)
        {
            kill(getppid(),SIGINT);
            kill(getpid(),SIGKILL);
        }
        
        else
        {
            kill(getppid(),SIGUSR2);
            kill(getpid(),SIGSTOP);
        }
            

    }
    
    destroyClk(false);
    
    return 0;
}
