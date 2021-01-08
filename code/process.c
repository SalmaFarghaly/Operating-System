#include "headers.h"
/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char * argv[])
{
    //signal(SIGSTOP,handler);
    //check..
    initClk();
    printf("process created.......%d\n",getpid());
    int oldtime = getClk();
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);    //Pass the remaining time as a parameter for the process
    while (remainingtime > 0)
    {

        if(oldtime+1 == getClk()){  //Check if 1 second has passed
            remainingtime--;        
            oldtime=getClk();     
            
        }
    }
    printf("sending termination signal .......%d to %d\n",getpid(),getppid());
    kill(getppid(),SIGUSR1);
    printf("termination signal sent.......%d to %d\n",getpid(),getppid());
    destroyClk(false);
    return 0;
}
