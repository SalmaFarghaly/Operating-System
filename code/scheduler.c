#include "headers.h"
#include "sys/msg.h"
#include "stdio.h"

#include "PriorityQueue.h"
#include "Queue.h"


// id of message queue betweeen scheduler and process generator
int PG2S_msqid;
//===========Variables used by Round Robin Function
int quantum=0;
bool p_running=false;
char number_str[10];
char number_str2[10];
int num_proc=0;


//===============Functions used by RoundRobin Function
void RoundRobin();
bool RR_allFinished(struct Queue*q);

//===========Variables used by HPF
void HPF();
bool HPF_isFinished = false;

void SRTN();

void handler(int signum);

int main(int argc, char * argv[])
{
    
    initClk();
    printf("\nSechedular: I have just begun clkk %d\n",getClk());
    key_t PG2S_key_id;
            

    // Create a message Queue to send info to scheduler.
    PG2S_key_id= ftok("keyfile", 1);
    PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
    struct algoMsgBuff message;
    int rec_val=msgrcv(PG2S_msqid , &message, sizeof(message.algo)+sizeof(message.quantum)+sizeof(message.num_proc),0, !IPC_NOWAIT);
    printf("\nScheduler : algo:%d,quantum:%d\n",message.algo,message.quantum);
    quantum=message.quantum;
    num_proc=message.num_proc;
    if(message.algo==1)
        RoundRobin();
    if(message.algo==2)
        SRTN();
    if(message.algo==3){
        printf("start HPF algo: %d \n",message.algo);
        HPF();
    }

    
}
//Function used by RoundRobin to check if the ready queue
//has unfinished process or not.
bool RR_allFinished(struct Queue*q){

    struct Node*cur_ptr=q->headPtr;
    while(cur_ptr!=NULL){
        if(cur_ptr->processObj->status!=-1){
            return false; // There is at least one process that hasn't finished yet.
        }
        cur_ptr=cur_ptr->nextNodePtr;
    }
    return true; // all processes have finished.

}

void RoundRobin(){
    
    struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
    struct Queue*RR_Queue=Queue_Constructor();
    int tempQuantum=quantum;
    struct process*cur_process;
    struct Node*cur_node;
    int y=0;
    int rec=1;
    int idx=0;

    while(1){
        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int rec_val=msgrcv(PG2S_msqid , &m, sizeof(m.val),0, !IPC_NOWAIT);
        int temp_val=m.val;
        if(rec!=-1)rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        while(temp_val!=0 && temp_val!=-1){
            rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),0, !IPC_NOWAIT);

            if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);
            }
            // Enqueue the recieved processes in RR_Queue.
            enqueue(RR_Queue,&arr[idx].p);
            idx++;
            // if this process is the first arrived one , we should place it at headptr.
            if(y==0){
                cur_node=RR_Queue->headPtr;
                cur_process=cur_node->processObj;
            }
            y+=1;
            temp_val-=1;

        }
        // if all processes in the Queue has finished state
        // and the scheduler recieves a message telling him that there is no upcoming processes.
        if(RR_allFinished(RR_Queue)==true&&rec==-1){
                printf("SCH::EXITING\n");
                exit(0);
        }
        //if there is no current process running and queue 
        //isn't empty, Then we will choose a process to run it.
        if(isEmpty(RR_Queue)==false&&p_running==false){
            // advance the prointer to get the first unfinished process
            cur_node=cur_node->nextNodePtr;
            struct Node*fir_ptr=cur_node;
            if(cur_node==NULL)
                cur_node=RR_Queue->headPtr;
            int rewind=0;
            while(cur_node->processObj->status==-1){
                cur_node=cur_node->nextNodePtr;
                if(cur_node==NULL){
                    cur_node=RR_Queue->headPtr;
                }
                else if(cur_node==fir_ptr){
                    rewind=1;
                    break;
                }
            }
            if(cur_node!=NULL)
            cur_process=cur_node->processObj;

        }
        if(RR_allFinished(RR_Queue)==false&&p_running==false){
            printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
            //fork a process and runs it giving it's remaining time as an argument
            if(cur_process->pid == -1){
                cur_process->pid=fork();
                int cur_pid=cur_process->pid;
                if(cur_pid==0){
                    sprintf(number_str,"%d",cur_process->remainingTime);
                    char*args[]={"./process.out",number_str};
                    execv(args[0],args);
                    exit(0);
                }
            }
            // if the process was alraedy forked but blocked 
            // then we should awake it.
            else{
                printf("\nProcess %d Resume at clk %d\n",cur_process->id,getClk());
                kill(cur_process->pid,SIGCONT);                

            }
                
            p_running=true;
            cur_process->status=1;
                    

        }
        // if there is a running process we should decrease it's remaining time by 1
        // and decrease quantum by 1
        if(p_running){
            printf("process %d is running,clk %d\n",cur_process->id,getClk());
            if(cur_process!=NULL){
                cur_process->remainingTime-=1;
                tempQuantum-=1;
                // if process has finished execution we should kill it
                // and set it's status to finished
                // and reset the quantum
                if(cur_process->remainingTime<=0){ // process finished execution
                    p_running = false;
                    cur_process->status=-1;
                    tempQuantum=quantum;
                    kill(cur_process->pid,SIGKILL);
                    printf("\nProcess %d Killed at clk %d\n",cur_process->id,getClk());
                    continue;
                }
            }
        }
        // if Quantum has ended , we should the running process,and schedule from
        // beginning to choose the appropiate process.
        if(tempQuantum<=0){ // process finished its quantum
            tempQuantum=quantum;
            p_running=false;
            kill(cur_process->pid,SIGSTOP);
            printf("\nProcess %d Paused at clk %d\n",cur_process->id,getClk());
            cur_process->status=0;
        }
    }
    printf("EXITING\n");
    exit(0);
    destroyClk(true);
    
}

void handler(int signum){
    printf("Termination signal received  .......................................\n");
    HPF_isFinished=true;
    signal(SIGUSR1,handler);
}

void HPF(){
    struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
    //struct Queue*RR_Queue=Queue_Constructor();

    int tempQuantum=quantum;
    struct process*cur_process=NULL;
    //struct Node*cur_node;
    struct pnode * pQHead=NULL;
    struct pnode * cur_node=NULL;
    int y=0;
    int rec=1;
    int idx=0;
    signal(SIGUSR1,handler);
    printf("my pid is %d....\n",getpid());
    while(1){
        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int rec_val=msgrcv(PG2S_msqid , &m, sizeof(m.val),0, !IPC_NOWAIT);
        int temp_val=m.val;
        if(rec!=-1)rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        while(temp_val!=0 && temp_val!=-1){
            rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),0, !IPC_NOWAIT);

            if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);
            }
            // Enqueue the recieved processes in RR_Queue.
            if(PQisEmpty(&pQHead)){
                pQHead = newNode(&arr[idx].p,arr[idx].p.priority);
            }else{
                push(&pQHead,&arr[idx].p,arr[idx].p.priority);
            }
            //enqueue(RR_Queue,&arr[idx].p);
            idx++;
            // if this process is the first arrived one , we should place it at headptr.
            if(y==0){
                cur_node=pQHead;
                cur_process=cur_node->processobj;
            }
            y+=1;
            temp_val-=1;

        }

       
        if(pQHead!=NULL){
            if (cur_process != NULL && HPF_isFinished==true){
                printf("Terminating the process........\n");
                cur_process->status = -1;
                pop(&pQHead);
                HPF_isFinished=false;
                cur_process = peek(&pQHead);
            }
            if(cur_process == NULL){
                cur_process = peek(&pQHead);
            }
            printf("current process = %d , status = %d\n",cur_process->id,cur_process->status);
            if(cur_process!=NULL && cur_process->status==0){
                printf("creating a process........\n");
                printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
                cur_process = peek(&pQHead);
                if(cur_process->pid == -1){
                cur_process->pid=fork();
                int cur_pid=cur_process->pid;
                    if(cur_pid==0){
                        sprintf(number_str,"%d",cur_process->remainingTime);
                        char*args[]={"./process.out",number_str,NULL};
                        execv(args[0],args);
                        exit(0);
                    }
                }
                p_running=true;
                cur_process->status=1;
            }
            // if (cur_process != NULL && HPF_isFinished==true){
            //     cur_process->status = -1;
            //     pop(&pQHead);
            //     HPF_isFinished=false;
            //     *cur_process = peek(&pQHead);
            // }
        }
        if(PQisEmpty(&pQHead)==true&&rec==-1){
            printf("SCH::EXITING\n");
            exit(0);
        }
    }
    printf("EXITING\n");
    exit(0);
    destroyClk(true);
}

void SRTN(){
    struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
    //struct Queue*RR_Queue=Queue_Constructor();

    int tempQuantum=quantum;
    struct process*cur_process=NULL;
    //struct Node*cur_node;
    struct pnode * pQHead=NULL;
    struct pnode * cur_node=NULL;
    int y=0;
    int rec=1;
    int idx=0;
    signal(SIGUSR1,handler);
    printf("my pid is %d....\n",getpid());
    
    while(1){
        // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
        // it will recieve -1 if there is no more processes to arrive later.
        struct msgbuff m;
        int rec_val=msgrcv(PG2S_msqid , &m, sizeof(m.val),0, !IPC_NOWAIT);
        int temp_val=m.val;
        if(rec!=-1)rec=m.val;
        // Thie while loop is responsible for recieving the processes'data.
        
        while(temp_val!=0 && temp_val!=-1){
            rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),0, !IPC_NOWAIT);

            if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);
            }
            // Enqueue the recieved processes in RR_Queue.
            if(PQisEmpty(&pQHead)){
                pQHead = newNode(&arr[idx].p,arr[idx].p.remainingTime);
            }else{
                push(&pQHead,&arr[idx].p,arr[idx].p.remainingTime);
            }
            //enqueue(RR_Queue,&arr[idx].p);
            idx++;
            // if this process is the first arrived one , we should place it at headptr.
            if(y==0){
                cur_node=pQHead;
                cur_process=cur_node->processobj;
                //cur_process->remainingTime++;
            }
            y+=1;
            temp_val-=1;

        }
        // if(cur_process!=NULL){
        //     cur_process->remainingTime=cur_process->remainingTime-1;
        //     printf("remainingtime=%d......\n",cur_process->remainingTime);
        // }
        
        if (cur_process != NULL && HPF_isFinished==true){
                printf("poping terminated process...\n");
                cur_process->status = -1;
                pop(&pQHead);
                HPF_isFinished=false;
                if(pQHead==NULL){
                    printf("no process available...\n");
                    cur_process=NULL;
                }else{
                    cur_process = peek(&pQHead);
                }
                if(cur_process==NULL){
                    printf("no process available...\n");
                }
                p_running=false;
                if(cur_process!=NULL){
                    //cur_process->remainingTime++;
                }
        }
        if(cur_process==NULL && pQHead!=NULL){
            printf("Here.......................\n");
            cur_node=pQHead;
            cur_process=cur_node->processobj;
            printf("Here.......................2\n");
        }
        if((pQHead!=NULL || p_running==false)&&cur_process!=NULL){
            printf("old process pid = %ld...remaining time = %d....\n",cur_process->pid,cur_process->remainingTime);
            struct process* temp = cur_process;
            cur_process = pQHead->processobj;
            printf("new process pid = %ld...remaining time = %d....\n",cur_process->pid,cur_process->remainingTime);
            
            if(temp->pid!=cur_process->pid || p_running == false){
                printf("entered...\n");
                if(p_running){
                    kill(temp->pid,SIGSTOP);
                    printf("process switch......................%ld, %d\n",temp->pid,getppid());
                }
                printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
                //fork a process and runs it giving it's remaining time as an argument
                if(cur_process->pid == -1){
                    cur_process->pid=fork();
                    int cur_pid=cur_process->pid;
                    if(cur_pid==0){
                        sprintf(number_str,"%d",cur_process->remainingTime);
                        char*args[]={"./process.out",number_str,NULL};
                        int err = execvp(args[0],args);
                        exit(0);
                    }
                }
                // if the process was alraedy forked but blocked 
                // then we should awake it.
                else{
                    printf("\nProcess %d Resume at clk %d\n",cur_process->id,getClk());
                    kill(cur_process->pid,SIGCONT);             

                }
                
                p_running=true;
                cur_process->status=1;
            }
        }
        if(cur_process!=NULL){
            cur_process->remainingTime=cur_process->remainingTime-1;
            pQHead->priority--;
            printf("remainingtime=%d......\n",cur_process->remainingTime);
        }

        // if(PQisEmpty(&pQHead)==false&&p_running==false){
        //     // advance the prointer to get the first unfinished process
        //     cur_node=cur_node->next;
        //     struct pnode*fir_ptr=cur_node;
        //     if(cur_node==NULL)
        //         cur_node=pQHead;
        //     int rewind=0;
        //     while(cur_node->processobj.status==-1){
        //         cur_node=cur_node->next;
        //         if(cur_node==NULL){
        //             cur_node=pQHead;
        //         }
        //         else if(cur_node==fir_ptr){
        //             rewind=1;
        //             break;
        //         }
        //     }
        //     if(cur_node!=NULL)
        //     *cur_process=cur_node->processobj;

        // }
        // if(PQisEmpty(&pQHead)==false&&p_running==false){
        //     printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
        //     //fork a process and runs it giving it's remaining time as an argument
        //     if(cur_process->pid == -1){
        //         cur_process->pid=fork();
        //         int cur_pid=cur_process->pid;
        //         if(cur_pid==0){
        //             sprintf(number_str,"%d",cur_process->remainingTime);
        //             char*args[]={"./process.out",number_str,NULL};
        //             execv(args[0],args);
        //             exit(0);
        //         }
        //     }
        //     // if the process was alraedy forked but blocked 
        //     // then we should awake it.
        //     else{
        //         printf("\nProcess %d Resume at clk %d\n",cur_process->id,getClk());
        //         kill(cur_process->pid,SIGCONT);                

        //     }
                
        //     p_running=true;
        //     cur_process->status=1;
                    

        // }
        // // if there is a running process we should decrease it's remaining time by 1
        // if(p_running){
        //     printf("process %d is running,clk %d\n",cur_process->id,getClk());
        //     if(cur_process!=NULL){
        //         cur_process->remainingTime-=1;
        //         // if process has finished execution we should kill it
        //         // and set it's status to finished
        //         // and reset the quantum
        //         if(HPF_isFinished==true){ // process finished execution
        //             p_running = false;
        //             HPF_isFinished=false;
        //             cur_process->status=-1;
        //             continue;
        //         }
        //     }
        // }
        
        if(PQisEmpty(&pQHead)==true&&rec==-1){
            printf("SCH::EXITING\n");
            exit(0);
        }
    }
    printf("EXITING\n");
    exit(0);
    destroyClk(true);
}


// void SRTN(){
//     struct msgProcess *arr=(struct msgProcess*)malloc(num_proc*sizeof(struct msgProcess));
//     //struct Queue*RR_Queue=Queue_Constructor();

//     int tempQuantum=quantum;
//     struct process*cur_process=NULL;
//     //struct Node*cur_node;
//     struct pnode * pQHead=NULL;
//     struct pnode * cur_node=NULL;
//     int y=0;
//     int rec=1;
//     int idx=0;
//     signal(SIGUSR1,HPFSIGCHLDHandler);
//     int oldtime=getClk();
//     bool firsttime=true;
//     while(1){
//         // Recieve a message first from the process_generator telling it how many processes will arrive in this clk cycle
//         // it will recieve -1 if there is no more processes to arrive later.
//         struct msgbuff m;
//         int rec_val=msgrcv(PG2S_msqid , &m, sizeof(m.val),0, !IPC_NOWAIT);
//         int temp_val=m.val;
//         if(rec!=-1)rec=m.val;
//         // Thie while loop is responsible for recieving the processes'data.
//         while(temp_val!=0 && temp_val!=-1){
//             rec_val=msgrcv(PG2S_msqid , &arr[idx], sizeof(arr[idx].p),0, !IPC_NOWAIT);

//             if(rec_val!=-1){  
//                     printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
//                                     ,arr[idx].p.id,arr[idx].p.arrivalTime,arr[idx].p.runTime,arr[idx].p.priority,arr[idx].p.memSize,arr[idx].p.pid);
//             }
//             // Enqueue the recieved processes in RR_Queue.
//             if(PQisEmpty(&pQHead)){
//                 pQHead = newNode(arr[idx].p,arr[idx].p.remainingTime);
//             }else{
//                 push(&pQHead,arr[idx].p,arr[idx].p.remainingTime);
//             }
//             //enqueue(RR_Queue,&arr[idx].p);
//             idx++;
//             // if this process is the first arrived one , we should place it at headptr.
//             if(y==0){
//                 cur_node=pQHead;
//                 cur_process=&cur_node->processobj;
//             }
//             y+=1;
//             temp_val-=1;

//         }

//         printf("finished receiving.... \n");
//         if(pQHead!=NULL){
//             if(cur_process != NULL){
//                 printf("enter... \n");
//                 cur_process->remainingTime=cur_process->remainingTime -1;
//                 struct process * temp;
//                 temp = cur_process;
//                 *cur_process = peek(&pQHead);
//                 printf("dbgg1... \n");
//                 if(cur_process!=temp||firsttime==true){
//                     printf("dbgg2... \n");
//                     firsttime=false;
//                     kill(temp->pid,SIGSTOP);
//                     printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
//                     //fork a process and runs it giving it's remaining time as an argument
//                     if(cur_process->pid == -1){
//                         cur_process->pid=fork();
//                         int cur_pid=cur_process->pid;
//                         if(cur_pid==0){
//                             sprintf(number_str,"%d",cur_process->remainingTime);
//                             char*args[]={"./process.out",number_str};
//                             execv(args[0],args);
//                             exit(0);
//                         }
//                     }
//                     // if the process was alraedy forked but blocked 
//                     // then we should awake it.
//                     else{
//                         printf("\nProcess %d Resume at clk %d\n",cur_process->id,getClk());
//                         kill(cur_process->pid,SIGCONT);                

//                     }
                        
//                     p_running=true;
//                     cur_process->status=1;
                
//             }
                
//                 //printf("current process = %d , status = %d\n",cur_process->id,cur_process->status);
    
                
//                 if (cur_process != NULL && HPF_isFinished==true){
//                     cur_process->status = -1;
//                     pop(&pQHead);
//                     HPF_isFinished=false;
//                     *cur_process = peek(&pQHead);
//                 }
//             }
//         }
//     }
//         if(PQisEmpty(&pQHead)==true&&rec==-1){
//             printf("SCH::EXITING\n");
//             exit(0);
//         }


// }

void HPFSIGUSR1Handler(int signum){
    printf("Termination signal received  .......................................\n");
    HPF_isFinished=true;
    signal(SIGUSR1,HPFSIGUSR1Handler);
}