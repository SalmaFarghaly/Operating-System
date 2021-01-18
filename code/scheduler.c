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
int WTA_idx=0;
int total_run=0;
//===============Functions used by RoundRobin Function
void RoundRobin(float*WTA_arr);
bool RR_allFinished(struct Queue*q);
struct process* findProcessWithPid(int pid);
//================Functions used by HPF 
void HPF(float*WTA_arr);

//================Functions used by SRTN
void SRTN(float*WTA_arr);

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
    float*WTA_arr=(float*)malloc(num_proc*sizeof(float));
    int start_clk=getClk();
    if(message.algo==1){
        RoundRobin(WTA_arr);
    }
    if(message.algo==2){
        SRTN(WTA_arr);
    }
    if(message.algo==3){
        HPF(WTA_arr);
    }
    int end_clk=getClk();
    //======calculate standard deviation====///
    float sum=0.0,mean,SD=0.0;
    for(int i=0;i<num_proc;i++){
        sum+=WTA_arr[i];
    }
    mean=sum/num_proc;
    for(int i=0;i<num_proc;i++){
        SD+=pow(WTA_arr[i]-mean,2);
        SD=sqrt(SD/num_proc);
    }
  
    //======for printing in .perf file=====/////

    fp = fopen("logs/scheduler.perf", "w");
    float u=(float)total_run/(float)(end_clk);
    printf("TOTALLL RUNNNN %f\n",u);
    u=roundf(u*100)/100;
    printf("TOTALLL RUNNNN %f\n",u);
    fprintf(fp,"CPU utilization = %.2f%%\n",u*100);
    float num=(float)total_TA;
    float dem=(float) num_proc;
    float result=num/dem;
    fprintf(fp,"Avg WTA = %.2f\n",roundf(result*100)/100);
    num=(float)total_wait;
    result=num/dem;
    printf("WTA %.2f ,WAIT %.2f\n",result,result);
    fprintf(fp,"Avg Waiting = %.2f\n",roundf(result*100)/100);
    fprintf(fp,"Std WTA = %.2f\n",roundf(SD*100)/100);
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
    printf("Termination signal received at clk:%d\n",getClk());
    if(signum==SIGUSR1){
        int stat_loc=0;
        int pid=wait(&stat_loc);
        if(!stat_loc&0x00FF){
                handler=1;
        }

    }
    signal(SIGUSR1,myhandler);
   
}

void HPF(float*WTA_arr){
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
                total_run++;
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
                WTA_arr[WTA_idx]=WTA;
                WTA_idx++;
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
            printf("MMMMMMMMMMMMSCH :: Error in receiving mssggggggggggggggg %d\n",getClk());
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
                printf("SCH :: Error in receiving mssggggggggggggggg %d\n",clk);
            }

        }


        //========= At beginning of a clk cycle======///
        // printf("BLOCKKKKKKK 11111\n");

        if(PQisEmpty(&pQHead)==true&&rec==-1){
            if(cur_process!=NULL){
                if(cur_process->status!=1){
                printf("SCH::EXITING\n");
                return;
                }
            }
            else{
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

        //  printf("BLOCKKKKKKK 22222\n");

        if(cur_process!=NULL){
            // printf("BLOCKKKKKKK 333333\n");
            if(cur_process->status!=1){ // stopped 0 finished -1 not started yet 0
            if(temp!=1){
                // printf("BLOCKKKKKKK 55555555555555\n");
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
            // printf("BLOCKKKKKKK 555555\n");
        }
        printf("CLKKKK CYCLEEEEEEE FINISHEDDDDDDDD !!!!!!!\n");
    }

}

void RoundRobin(float*WTA_arr){

    
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
                total_run++;
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
                WTA_arr[WTA_idx]=WTA;
                WTA_idx++;
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
            printf("MMMMMMMMMMMMSCH :: Error in receiving mssggggggggggggggg %d\n",getClk());
        }
        int temp_val=m.val;
        rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        printf("SCHHHHH:: m.val %d , clk %d\n",m.val,getClk());
        while(temp_val!=0 && temp_val!=-1){
            printf("IN LOOP :: SCHHHHH:: m.val %d , clk %d\n",m.val,getClk());
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
                        printf("179::::NULLLLLLLLLLLLLLLLLLLLLLLLLLLL\n");
                        cur_process=cur_node->processObj;
                    }
                    y+=1;
                    temp_val-=1;
            }
            else{
                int clk=getClk();
                printf("SCH :: Error in receiving mssggggggggggggggg %d\n",clk);
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
                            printf("SCH::SENTTTTTT SIGNALLLL clk %d STOP TO pid %ld",getClk(),prev_process->pid); 
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
        printf("CLKKKK CYCLEEEEEEE FINISHEDDDDDDDD !!!!!!!\n");
        print_Queue(RR_Queue);
    }


}



void SRTN(float*WTA_arr){
    fp = fopen("logs/SRTN/scheduler2.log", "w");
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
                total_run++;
                pQHead->priority--;
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
                printf("poping process......\n");
                float num=(float)getClk()-cur_process->arrivalTime;
                float dem=(float)cur_process->runTime;

                float result=num/dem;
                float WTA=ceilf(result*100)/100;
                WTA_arr[WTA_idx]=WTA;
                WTA_idx++;
                total_wait+=cur_process->wait_time;
                total_TA+=getClk()-cur_process->arrivalTime;
                fp = fopen("logs/SRTN/scheduler2.log", "a");
                fprintf(fp, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n",
                getClk(),cur_process->id
                ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,
                cur_process->wait_time,getClk()-(cur_process->arrivalTime),WTA);
                    fclose(fp);
                cur_process->status=-1;
                pop(&pQHead);
                cur_process=NULL;
                handler=0;
            }
        }

        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int r=msgrcv(M_PG2S_msqid , &m, sizeof(m.val),sch, !IPC_NOWAIT);
        if(r==-1){
            printf("MMMMMMMMMMMMSCH :: Error in receiving mssggggggggggggggg %d\n",getClk());
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
                        pQHead=newNode(&arr[idx].p,arr[idx].p.runTime);
                    }
                    else{
                        push(&pQHead,&arr[idx].p,arr[idx].p.runTime);
                    }
                    idx++;

                    temp_val-=1;
            }
            else{
                int clk=getClk();
                printf("SCH :: Error in receiving mssggggggggggggggg %d\n",clk);
            }

        }


        //========= At beginning of a clk cycle======///
        // printf("BLOCKKKKKKK 11111\n");

        if(PQisEmpty(&pQHead)==true&&rec==-1){
            if(cur_process!=NULL){
                if(cur_process->status!=1){
                printf("SCH::EXITING\n");
                return;
                }
            }
            else{
                return;
            }
            
        }
        //if there is process that should be stopped;
        //-1 cur has finished or number pid to stop it
        int temp=0;
        if(cur_process==NULL && PQisEmpty(&pQHead)==false){
            printf("getting new process...\n");
            cur_process=peek(&pQHead);
            
        }

        //  printf("BLOCKKKKKKK 22222\n");

        if(cur_process!=NULL){
            printf("BLOCKKKKKKK 333333\n");
            struct process * temp_p= cur_process;
            cur_process = peek(&pQHead);

            printf("process ID:%d ,  status: %d....\n",cur_process->id,cur_process->status);


            if(cur_process->pid!=temp_p->pid){
                printf("Enter 1.......................\n");
                int k=kill((pid_t)temp_p->pid,SIGUSR2);
                if(k==-1)
                    perror("Error in stopping signal\n"); 
                else
                    printf("SCH::SENTTTTTT SIGNALLLL clk %d STOP TO pid %ld",getClk(),temp_p->pid); 
                fp = fopen("logs/SRTN/scheduler2.log", "a");
                fprintf(fp, "At time\t%d\tprocess\t%d\tstopped\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),temp_p->id
                ,temp_p->arrivalTime,temp_p->runTime,temp_p->remainingTime,temp_p->wait_time);
                // fflush(fp);
                fclose(fp);
                printf("\nProcess %d Paused at clk %d\n",temp_p->id,getClk());
                temp_p->lstfinish_time=getClk(); 
                temp_p->status=0;
            }
            

            if(cur_process->pid==-1){
                cur_process->wait_time=getClk()-cur_process->arrivalTime;
                printf("\nSCH::1Process %d starts at clk %d\n",cur_process->id,getClk());
                    fp = fopen("logs/SRTN/scheduler2.log", "a");
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
            }else{
                if(cur_process->status==0){
                    cur_process->status=1;
                    cur_process->wait_time+=getClk()-cur_process->lstfinish_time;
                    fp = fopen("logs/SRTN/scheduler2.log", "a");
                    fprintf(fp, "At time\t%d\tprocess\t%d\tresumed\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),cur_process->id
                    ,cur_process->arrivalTime,cur_process->runTime,cur_process->remainingTime,cur_process->wait_time);
                    // fflush(fp);
                        fclose(fp);
                    int k=kill((pid_t)cur_process->pid,SIGCONT);     
                    
                    if(k==-1)
                        perror("Error in resuming signal\n");
                    else
                        printf("\nSCH::1Process %d Resume at clk %d\n",cur_process->id,getClk());
                    }
            }
            
                
            
            // printf("BLOCKKKKKKK 555555\n");
        }
        printf("CLKKKK CYCLEEEEEEE FINISHEDDDDDDDD !!!!!!!\n");

}
}