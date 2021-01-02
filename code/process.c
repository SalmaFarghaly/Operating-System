#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    //check..
    initClk();
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
    
    destroyClk(false);
    
    return 0;
}
