#include "headers.h"
/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{

    //check..
    initClk();
    kill(getppid(),SIGUSR1);
    //printf("child running.... \n");
    int oldtime = getClk();
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);    //Pass the remaining time as a parameter for the process
    while (remainingtime > 0)
    {
        printf("child running: remaing time = %d.... \n",remainingtime);
        if(oldtime+1 == getClk()){  //Check if 1 second has passed
            remainingtime--;        
            oldtime=getClk();       
        }
    }

    kill(getppid(),SIGCHLD);
    
    destroyClk(false);
    
    return 0;
}
