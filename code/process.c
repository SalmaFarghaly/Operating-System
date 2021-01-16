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

// if(signum==SIGTSTP){
        lastTime=getClk();
        stoph=1;
        // raise(SI)
        // printf("\n Process:: pid %d STOPPPP at clk %d wait %d last %d remainingtime %d  \n",getpid(),getClk(),p_wait,lastTime,remainingtime);
// }

}

void handler(int signum){

    // if(signum==SIGCONT){

        p_wait+=getClk()-lastTime;
        stoph=0;
        // printf("\n Process:: pid %d CONTTTT at clk %d wait %d last %d remainingtime %d  \n",getpid(),getClk(),p_wait,lastTime,remainingtime);
    // }

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
    // lastTime=getClk();

    // int l = 0;  printf("P::BEGIN at time %d pid %d ,remaining time %d\n",getClk(),getpid(),remainingtime);
    while (remainingtime > 0)
    {
        // printf("P::at time %d pid %d run %d init %d p_Wait%d\n",getClk(),getpid(),run,remainingtime);

        remainingtime=run-getClk()+init+p_wait;
        if(stoph==1){
            stoph=0;
            raise(SIGSTOP);
        }
        // lastTime=getClk();
        // if(l!=getClk())
    //    printf("P::at time %d pid %d ,remaining time %d\n",getClk(),getpid(),remainingtime);
    //    l=getClk();
    }
    // lasttime!=getclk()-1
   
    // printf("child::remainingtimee%d clk %d\n",remainingtime,getClk());
    kill(getppid(),SIGUSR1);
    destroyClk(false);
    exit(0);
}
