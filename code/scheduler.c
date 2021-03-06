// #include "headers.h"
#include "Queue.h"
#include "PriorityQueue.h"
#include "sys/msg.h"
#include "stdio.h"
#include <math.h>


// id of message queue betweeen scheduler and process generator
int PG2S_msqid;
int M_PG2S_msqid;
//===========Variables used by Round Robin Function
int quantum=0;
bool p_running=false;
char number_str[10];
int num_proc=0;
int total_wait=0;
int total_TA=0;
int tempQuantum=0;
int rcv_processes=0;
int handler=0;
int prev_clk=-1;
int cur_clk=0;
long sch=0;
//===============Functions used by RoundRobin Function
void RoundRobin();
bool RR_allFinished(struct Queue*q);
struct process* findProcessWithPid(int pid);
//================Functions used by HPF 
void HPF();

//=============logs=======
FILE*fp;

//==============General Utilities======///
void myhandler();


int main(int argc, char * argv[]){


    signal(SIGUSR1,myhandler);//handling the exit signal from the child process.
    initClk();
    key_t PG2S_key_id_2;
    PG2S_key_id_2=ftok("keyfile",2);
    M_PG2S_msqid = msgget(PG2S_key_id_2, IPC_CREAT | 0644);
    if (M_PG2S_msqid== -1){
        perror("Error in create message queue to Scheduler\n");
        exit(-1);
    }
    
    key_t PG2S_key_id;
            

    // Create a message Queue to send info to scheduler.
    PG2S_key_id= ftok("keyfile", 1);
    PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
    struct algoMsgBuff message;
    int rec_val=msgrcv(PG2S_msqid , &message, sizeof(message.algo)+sizeof(message.quantum)+sizeof(message.num_proc),0, !IPC_NOWAIT);
    printf("\nScheduler : algo:%d,quantum:%d\n",message.algo,message.quantum);
    quantum=message.quantum;
    tempQuantum=quantum;
    num_proc=message.num_proc;
    if(message.algo==1){
        RoundRobin();
    }
    if(message.algo==3){
        HPF();
    }
  

    
 
    //======for printing in .perf file=====/////
    fclose(fp);
    fp = fopen("logs/Round_Robin/scheduler2.perf", "w");
    float num=(float)total_TA;
    float dem=(float) num_proc;
    float result=num/dem;
    fprintf(fp,"Avg WTA = %.2f\n",roundf(result*100)/100);
    num=(float)total_wait;
    result=num/dem;
    printf("WTA %.2f ,WAIT %.2f\n",result,result);
    fprintf(fp,"Avg Waiting = %.2f\n",roundf(result*100)/100);
    printf("WTA %.2f ,WAIT %.2f\n",result,result);
    fclose(fp);
    destroyClk(true);
    exit(0);

    
}
//Function used by RoundRobin to check if the ready queue
//has unfinished process or not.
bool RR_allFinished(struct Queue*q){

    struct Node*cur_ptr=q->headPtr;
    while(cur_ptr!=NULL){
        if(cur_ptr->processObj->remainingTime>0){
        
            return false; // There is at least one process that hasn't finished yet.
        }
        cur_ptr=cur_ptr->nextNodePtr;
    }
    return true; // all processes have finished.

}
//Function to handle exit signal of child process.
void myhandler(int signum){

    if(signum==SIGUSR1){
        int stat_loc=0;
        int pid=wait(&stat_loc);
        if(!stat_loc&0x00FF){
                handler=1;
        }

    }
    signal(SIGUSR1,myhandler);
   
}

void HPF(){
    fp = fopen("logs/HPF/scheduler2.log", "w");
    printf("\nSechedular: I have just begun clkk %d\n",getClk());
    fprintf(fp, "#At time\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\t\n");
    fflush(fp);
    fclose(fp);
    struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
    struct process*cur_process=NULL;
    //struct Node*cur_node;
    struct pnode * pQHead=NULL;
    struct pnode * cur_node=NULL;
    int y=0;
    int rec=1;
    int idx=0;
    int sch=getpid();
    signal(SIGUSR1,myhandler);
    printf("my pid is %d....\n",getpid());
    while(1){
        printf("SCH::Clkkk Cycle %d\n",getClk());
        if(cur_process!=NULL){
            if(cur_process->status==1){
                cur_process->remainingTime-=1;
                if(cur_process->remainingTime>0){
                }
                else{
                    if(handler==0){
                        sleep(5);
                        printf("clk :: %d process %d finished \n",getClk(),cur_process->id);
                        cur_process->status=-1;
                    }

                }
            }
            if(handler==1){
                float num=(float)getClk()-cur_process->arrivalTime;
                float dem=(float)cur_process->runTime;

                float result=num/dem;
                float WTA=ceilf(result*100)/100;
                total_wait+=cur_process->wait_time;
                total_TA+=getClk()-cur_process->arrivalTime;
                fp = fopen("logs/HPF/scheduler2.log", "a");
                fprintf(fp, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n",
                getClk(),cur_process->id
                ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,
                cur_process->wait_time,getClk()-(cur_process->arrivalTime),WTA);
                    fclose(fp);
                cur_process->status=-1;
                handler=0;
            }
        }

        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int r=msgrcv(M_PG2S_msqid , &m, sizeof(m.val),sch, !IPC_NOWAIT);
        if(r==-1){
            printf("sCH :: Error in receiving msg %d\n",getClk());
        }
        int temp_val=m.val;
        rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        printf("SCHHHHH:: m.val %d , clk %d\n",m.val,getClk());
        while(temp_val!=0 && temp_val!=-1){
            printf("IN LOOP :: SCHHHHH:: m.val %d , clk %d\n",m.val,getClk());
            int rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),sch, !IPC_NOWAIT);
            if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);


                    printf("\nENQUEUEU:::SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                            ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);
                  
                    if(PQisEmpty(&pQHead)){
                        pQHead=newNode(&arr[idx].p,arr[idx].p.priority);
                    }
                    else{
                        push(&pQHead,&arr[idx].p,arr[idx].p.priority);
                    }
                    idx++;

                    temp_val-=1;
            }
            else{
                int clk=getClk();
                printf("SCH :: Error in receiving msg %d\n",clk);
            }

        }


        //========= At beginning of a clk cycle======///
        // printf("BLOCKKKKKKK 11111\n");

        if(PQisEmpty(&pQHead)==true&&rec==-1){
            if(cur_process->status!=1){
            printf("SCH::EXITING\n");
            return;
            }
            
        }
        //if there is process that should be stopped;
        //-1 cur has finished or number pid to stop it
        int temp=0;
        if(cur_process==NULL && PQisEmpty(&pQHead)==false){
            cur_process=peek(&pQHead);
            pop(&pQHead);
            temp=1;
        }

        

        if(cur_process!=NULL){
            
            if(cur_process->status!=1){ // stopped 0 finished -1 not started yet 0
            if(temp!=1){
                
                cur_process=peek(&pQHead);
                pop(&pQHead);
            }
            printf("clk :: %d process %d started \n",getClk(),cur_process->id);
            cur_process->wait_time=getClk()-cur_process->arrivalTime;
            printf("\nSCH::Process %d starts at clk %d\n",cur_process->id,getClk());
            fp = fopen("logs/HPF/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tprocess\t%d\tstarted\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),cur_process->id
            ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,cur_process->wait_time);
            fflush(fp);
            fclose(fp);
                if(cur_process->pid==-1){
                    cur_process->pid=fork();
                    int cur_pid=cur_process->pid;
                    if(cur_pid==0){
                        sprintf(number_str,"%d",cur_process->remainingTime);
                        char*args[]={"./process.out",number_str,NULL};
                        execv(args[0],args);
                        exit(0);
                    }
                    cur_process->status=1;
                }
            }
            
        }
        
    }

}

void RoundRobin(){

    
    sch=getpid();
    struct Queue*RR_Queue=Queue_Constructor();

    struct process*cur_process=NULL;
    struct Node*cur_node=NULL;
    struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
    int y=0;
    int rec=1;
    int idx=0;
    fp = fopen("logs/Round_Robin/scheduler2.log", "w");
    printf("\nSechedular: I have just begun clkk %d\n",getClk());
    fprintf(fp, "#At time\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\t\n");
    fflush(fp);

    while(1){


        printf("SCH::Clkkk Cycle %d\n",getClk());
        if(cur_process!=NULL){
            if(cur_process->status==1){
                cur_process->remainingTime-=1;
                if(cur_process->remainingTime>0){
                    tempQuantum-=1;
                    if(tempQuantum<=0){
                        tempQuantum=quantum;
                        cur_process->status=0;  
                    }
                }
                else{
                    if(handler==0){

                        sleep(5);

                        cur_process->status=-1;
                    }

                }
            }
            if(handler==1){

                cur_process->status=-1;
                tempQuantum=quantum;
                float num=(float)getClk()-cur_process->arrivalTime;
                float dem=(float)cur_process->runTime;

                float result=num/dem;
                float WTA=ceilf(result*100)/100;
                total_wait+=cur_process->wait_time;
                total_TA+=getClk()-cur_process->arrivalTime;
                fp = fopen("logs/Round_Robin/scheduler2.log", "a");
                fprintf(fp, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n",
                getClk(),cur_process->id
                ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,
                cur_process->wait_time,getClk()-(cur_process->arrivalTime),WTA);
                    fclose(fp);
                handler=0;
            }
        }

        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int r=msgrcv(M_PG2S_msqid , &m, sizeof(m.val),sch, !IPC_NOWAIT);
        if(r==-1){
            printf("SCH :: Error in receiving msg %d\n",getClk());
        }
        int temp_val=m.val;
        rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        printf("SCH:: m.val %d , clk %d\n",m.val,getClk());
        while(temp_val!=0 && temp_val!=-1){
            printf("IN LOOP :: SCH:: m.val %d , clk %d\n",m.val,getClk());
            rcv_processes+=1;
            int rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),sch, !IPC_NOWAIT);
            if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);

                            // Enqueue the recieved processes in RR_Queue.
                    printf("Before Enqueue index %d\n",idx);
                    print_Queue(RR_Queue);

                    printf("\nENQUEUEU:::SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                            ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);

                    enqueue(RR_Queue,&arr[idx].p);
                    idx++;
                    printf("\nAfter Enqueue\n");
                    print_Queue(RR_Queue);

                    // if this process is the first arrived one , we should place it at headptr.
                    if(y==0){
                        cur_node=RR_Queue->headPtr;
                        if(cur_node==NULL)
                        
                        cur_process=cur_node->processObj;
                    }
                    y+=1;
                    temp_val-=1;
            }
            else{
                int clk=getClk();
                printf("SCH :: Error in receiving msg %d\n",clk);
            }

        }


        //========= At beginning of a clk cycle======///

        if(RR_allFinished(RR_Queue)&&rec==-1){
            printf("SCH::EXITING\n");
            return;
        }
        //if there is process that should be stopped;
        //-1 cur has finished or number pid to stop it

        if(cur_process!=NULL){
            if(cur_process->status!=1){ // stopped 0 finished -1 not started yet 0
                struct process*prev_process=dequeue(RR_Queue);
                cur_process=peekFront(RR_Queue);
                if(cur_process==NULL){
                    enqueue(RR_Queue,prev_process);
                    cur_process=prev_process;
                }
                if(cur_process->pid==-1){
                    cur_process->wait_time=getClk()-cur_process->arrivalTime;
                    printf("\nSCH::Process %d starts at clk %d\n",cur_process->id,getClk());
                      fp = fopen("logs/Round_Robin/scheduler2.log", "a");
                    fprintf(fp, "At time\t%d\tprocess\t%d\tstarted\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),cur_process->id
                    ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,cur_process->wait_time);
                    // fflush(fp);
                    fclose(fp);
                    cur_process->pid=fork();
                    int cur_pid=cur_process->pid;
                    if(cur_pid==0){
                        sprintf(number_str,"%d",cur_process->remainingTime);
                        char*args[]={"./process.out",number_str,NULL};
                        execv(args[0],args);
                        exit(0);
                    }
                    cur_process->status=1;
                }
                else{
                    cur_process->status=1;
                    if(prev_process->pid!=cur_process->pid){
                    cur_process->wait_time+=getClk()-cur_process->lstfinish_time;
                    fp = fopen("logs/Round_Robin/scheduler2.log", "a");
                    fprintf(fp, "At time\t%d\tprocess\t%d\tresumed\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),cur_process->id
                    ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,cur_process->wait_time);
                    // fflush(fp);
                        fclose(fp);
                    int k=kill((pid_t)cur_process->pid,SIGCONT);     
                    if(k==-1)
                        perror("Error in resuming signal\n");
                    else
                        printf("\nSCH::Process %d Resume at clk %d\n",cur_process->id,getClk());
                    }
                    
                }
                if(prev_process!=NULL){
                    if(prev_process->status==0){
                            //send stop to prev and resume to cur
                        if(prev_process->pid!=cur_process->pid){
                        int k=kill((pid_t)prev_process->pid,SIGUSR2);
                        if(k==-1)
                            perror("Error in stopping signal\n"); 
                        else
                            printf("SCH::sent signal clk %d STOP TO pid %ld",getClk(),prev_process->pid); 
                        fp = fopen("logs/Round_Robin/scheduler2.log", "a");
                        fprintf(fp, "At time\t%d\tprocess\t%d\tstopped\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),prev_process->id
                        ,prev_process->arrivalTime,prev_process->runTime,prev_process->remainingTime,prev_process->wait_time);
                        // fflush(fp);
                        fclose(fp);
                        printf("\nProcess %d Paused at clk %d\n",prev_process->id,getClk());
                        prev_process->lstfinish_time=getClk(); 
                        enqueue(RR_Queue,prev_process);
                        }
 
                    }
                }
            }
        }
       
        print_Queue(RR_Queue);
    }


}
