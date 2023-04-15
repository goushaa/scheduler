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
    printf("id: %d forked with remainingTime: %d & quantum %d\n",getpid(),remainingTime,quantum);

    key_t pKey = ftok("key", 's');
    int remainingshmid = shmget(pKey,4, 0);

    if (remainingshmid == -1)
        printf("Error in create\n");
    else
        printf("\nShared memory ID = %d\n", remainingshmid);

    int *remshmaddr = (int*) shmat(remainingshmid, (void *)0, 0);
    if (remshmaddr == (void *)-1)
    {
        printf("Error in attach in process %d\n",getpid());
    }

    struct sembuf rem_sem_op;
    key_t sKey = ftok("key", 'd');
    int semid = semget(sKey, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        printf("Error semid create in process\n");
    }
    int finished = 0;
    while (remainingTime > 0)
    {
        int previousTime = getClk();
        if(remainingTime <= quantum)
        {
            quantum = remainingTime; 
            int finished = 1;
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
        *remshmaddr = remainingTime;
        //up//
        rem_sem_op.sem_num = 0;
        rem_sem_op.sem_op = 1;
        rem_sem_op.sem_flg = !IPC_NOWAIT;
        semop(semid, &rem_sem_op, 1);
        //up//
        if(finished == 1)
        {
            printf("id:%d mot 3and w fadely wa2t %d\n",getpid(),remainingTime);
            shmdt(remshmaddr);
            exit(0);
        } 
        else
        {
            printf("id:%d 5alast 7etety w fadely wa2t %d\n",getpid(),remainingTime);
            kill(getpid(),SIGSTOP);
        }   
            
    }
    
    destroyClk(false);
    
    return 0;
}
