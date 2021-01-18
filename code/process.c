#include "headers.h"
#include "time.h"
/* Modify this file as needed*/
int remainingtime;
int p_wait=0;
int run=0;
int lastTime=0;
int stoph=0;
time_t now=0;
struct tm*tm;
int prev_clk=-1;

void stop_handler(int signum){


        lastTime=getClk();
        stoph=1;

}

void handler(int signum){



        p_wait+=getClk()-lastTime;
        stoph=0;

}


int main(int agrc, char * argv[])
{
    //check..
    initClk();
    signal(SIGUSR2,stop_handler);
    signal(SIGCONT,handler);
    // int oldtime = getClk();
    int init=getClk();
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);    //Pass the remaining time as a parameter for the process
    run=remainingtime;
    
    while (remainingtime > 0)
    {
        

        remainingtime=run-getClk()+init+p_wait;
        if(stoph==1){
            stoph=0;
            raise(SIGSTOP);
        }

    }

   
    kill(getppid(),SIGUSR1);
    destroyClk(false);
    exit(0);
}
