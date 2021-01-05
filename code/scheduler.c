// #include "headers.h"
#include "Queue.h"
#include "sys/msg.h"
#include "stdio.h"


int PG2S_msqid;
int cur_clk=-1;
int quantum=0;
int rc=0;
bool p_running=false;
struct msqid_ds buf;
char number_str[10];
void RoundRobin();
bool RR_allFinished(struct Queue*q);
struct msgProcess arr[4];

int main(int argc, char * argv[])
{
    
    initClk();
    printf("\nSechedular: I have just begun clkk %d\n",getClk());
    key_t PG2S_key_id;
            

    // Create a message Queue to send info to scheduler.
    PG2S_key_id= ftok("keyfile", 1);
    PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
    struct algoMsgBuff message;
    int rec_val=msgrcv(PG2S_msqid , &message, sizeof(message.algo)+sizeof(message.quantum),0, !IPC_NOWAIT);
    printf("\nScheduler : algo:%d,quantum:%d\n",message.algo,message.quantum);
    quantum=message.quantum;


    
    struct Queue*RR_Queue=Queue_Constructor();
    int cur_clk=getClk();
    int prev_clk=-1;
    int cycle_num=0;
    int k=0;
    int tempQuantum=quantum;
    struct process*cur_process;//=NULL;//=RR_Queue->headPtr->processObj;
    struct Node*cur_node;//=NULL;//RR_Queue->headPtr;
    int y=0;
    int rec=1;
    int o=0;
    while(1){
        if(rec!=-1){
            struct msgbuff m;
            int rec_val=msgrcv(PG2S_msqid , &m, sizeof(m.val),0, !IPC_NOWAIT);
            // printf("SCH::I recieved %d clk %d\n",m.val,getClk());
            int temp_val=m.val;
            rec=m.val;
            while(temp_val!=0 && temp_val!=-1){
                if(cur_process!=NULL)
                    printf("Beforeeeeeeee PPPPPPPPPPPPPPPPPP %d\n",cur_process->id);
        
                printf("Structt  ADDRESSSS %p\n",&arr[o]);
                rec_val=msgrcv(PG2S_msqid , &arr[o], sizeof(arr[o].p),0, !IPC_NOWAIT);

                if(rec_val!=-1){  
                    printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,arr[o].p.id,arr[o].p.arrivalTime,arr[o].p.runTime,arr[o].p.priority,arr[o].p.memSize,arr[o].p.pid);
                }
                // printf("AFTEEEEEERRRRR ENQUEEEEEEE %d\n",cur_process->id);
                if(cur_process!=NULL){
                        printf("Beforeeeeeeee ENQUEEEEEEE %d\n",cur_process->id);
                        //  printf("ADDRESSSS &%p",&p.p);
                       
                }
                enqueue(RR_Queue,&arr[o].p);
                o++;
                

                if(y==0){
                    cur_node=RR_Queue->headPtr;
                    cur_process=cur_node->processObj;
                    // printf("FFFFFFFFFFFFFFFFFFFF\n");
                    //  printf("ADDRESSSS &%p",&p.p);
                }
                printf("AFTEEEEEERRRRR ENQUEEEEEEE %d\n",cur_process->id);
                y+=1;
                temp_val-=1;

            }
        }
        // printf("prunninngggggggggggg %d\n",p_running);
        if(RR_allFinished(RR_Queue)==false&&p_running==false){
            printf("Process %d will run ,clk %d\n",cur_process->id,getClk());
            cur_process->pid=fork();
            int cur_pid=cur_process->pid;
            if(cur_pid==0){
                sprintf(number_str,"%d",cur_process->remainingTime);
                char*args[]={"./process.out",number_str};
                execv(args[0],args);
                exit(0);
            }
            
            p_running=true;
            cur_process->status=1;
                 

        }
        if(p_running){
            printf("process %d is running,clk %d\n",cur_process->id,getClk());
            if(cur_process!=NULL){
                cur_process->remainingTime-=1;
                tempQuantum-=1;
                printf("Quantummmmmmm%d,clkkkk%d\n",tempQuantum,getClk());
                if(cur_process->remainingTime==0){
                    // printf("Remaining Tirmrrr")
                    cur_process->status=-1;
                    cur_node=cur_node->nextNodePtr;
                    if(cur_node!=NULL){
                        while(cur_node->processObj->status==-1){
                            if(cur_node==NULL)break;
                            cur_node=cur_node->nextNodePtr;
                        }
                    }
                    if (cur_node==NULL){
                        cur_node=RR_Queue->headPtr;
                    }
                }
            }
        }
        if(tempQuantum==0){
            printf("pppppppppppppppppppppppppppp\n");
            tempQuantum=quantum;
            p_running=false;
            cur_node=cur_node->nextNodePtr;
            if(cur_node!=NULL){
                while(cur_node->processObj->status==-1){
                    if(cur_node==NULL)break;
                    cur_node=cur_node->nextNodePtr;
                }
            }
            if (cur_node==NULL){
                cur_node=RR_Queue->headPtr;
            }
        }
    }
    
    destroyClk(true);
}

bool RR_allFinished(struct Queue*q){

    struct Node*cur_ptr=q->headPtr;
    while(cur_ptr!=NULL){
        if(cur_ptr->processObj->status!=-1)
            return false;
    }
    return true;

}
// void RoundRobin(){
    





// }