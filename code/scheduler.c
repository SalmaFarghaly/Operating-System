#include "headers.h"



int PG2S_msqid;
int cur_clk=-1;

int main(int argc, char * argv[])
{
    printf("\nSechedular: I have just begun my pid %d\n",getpid());
    initClk();
    key_t PG2S_key_id;
            

                // Create a message Queue to send info to scheduler.
    PG2S_key_id= ftok("keyfile", 1);
    PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
    struct algoMsgBuff message;
    int rec_val=msgrcv(PG2S_msqid , &message, sizeof(message.algo)+sizeof(message.quantum),0, !IPC_NOWAIT);
    // printf("\nScheduler : algo:%d,quantum:%d\n",message.algo,message.quantum);

    while(1){

        struct msgProcess p;
        int rec_val=msgrcv(PG2S_msqid , &p, sizeof(p.p),0, !IPC_NOWAIT);
        printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                        ,p.p.id,p.p.arrivalTime,p.p.runTime,p.p.priority,p.p.memSize,p.p.pid);

    }
    
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
