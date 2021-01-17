#include "headers.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"
#include <signal.h>
#include <stdio.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/stat.h>



#define MAXCHAR 1000
int clk_pid;       // id of clock process
int sch_pid;       // id of scheduler process
int PG2S_msqid;    // id of message queue between Scheduler and Process generator.
int M_PG2S_msqid;
char myname[19]="Process Generator: ";
int  proc_cnt=0;  // number of processes
int cur_clk=-1;
int prev_clk=-1;



void clearResources();
void readProcessesData(char*file_name,struct process*processes);
void splitTokens(char*str,struct process*pro);
void getProcessesCnt();

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // 1. Read the input files.
    char*file_name="testcases/processes2.txt";
    getProcessesCnt(file_name);
    struct process*processes=(struct process*)malloc(proc_cnt*sizeof(struct process));
    readProcessesData(file_name,processes);
    
    //========= Do not remove this part , it is just a printing part for checking and debugging=====////
    // printf("%s\n",myname);
    // printf("The number of Processes:%d\n",proc_cnt);
    // for(int k=0;k<proc_cnt;k++){
    //     struct process p=processes[k];
    //     printf("process no.%d,id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",k,p.id,p.arrivalTime,p.runTime,p.priority,p.memSize,p.pid);
    // }
    //==============================================================================================////


 
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int quantum=0, choice=0;
    printf("\nChoose the scheduling algorithm:  \n");
    printf("1.Round Robin \n");
    printf("2.SRTN \n");
    printf("3.Nonpreemptive HPF \n");
    printf("4.Quit \n\n");
    printf("Enter your choice : \n");

    scanf("%d", &choice);
    switch (choice){
        case 1:
            printf("\nEnter the quantum: \n");
            scanf("%d", &quantum);
            break;
        case 4:
            exit(1);
            break;
        default:
            break;
    }

    // 3. Initiate and create the scheduler and clock processes.
    clk_pid=fork();
    // This will be executed by the clk child process
    if(clk_pid==0){
        // printf("I am the child my pid : %d\n",getpid());
        char*args[]={"./clk.out",NULL};
        execvp(args[0],args);
        exit(0);
    }
    else if(clk_pid==-1){
        printf("Error in creating clock process");
        exit(0);
    }
    //This block will be executed by the parent(process generator)
    else{
        // 4. Use this function after creating the clock process to initialize clock
            initClk();
            sch_pid=fork();
            if (sch_pid==0){
                // printf("PG::FOrking sch\n");
                char*args[]={"./scheduler.out",NULL};
                execv(args[0],args);
                exit(0);
            }
            else{
                key_t PG2S_key_id;
                key_t PG2S_key_id_2;
            

                // Create a message Queue to send info to scheduler.
                PG2S_key_id= ftok("keyfile", 1);
                PG2S_key_id_2=ftok("keyfile",2);
                PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
                if (PG2S_msqid == -1){
                    printf("%s\n",myname);
                    perror("Error in create message queue to Scheduler\n");
                    exit(-1);
                }
                M_PG2S_msqid = msgget(PG2S_key_id_2, IPC_CREAT | 0644);
                if (M_PG2S_msqid== -1){
                    printf("%s\n",myname);
                    perror("Error in create message queue to Scheduler\n");
                    exit(-1);
                }
                // printf("message Queue:%d\n",PG2S_msqid);


                // Send which Algo to run as a message through queue to the scheduler
                struct algoMsgBuff  message;
                message.algo=choice;
                message.quantum=quantum;
                message.mtype=sch_pid%10000;
                message.num_proc=proc_cnt;

                // Send message to the scheduler contains which algorithm to run and the parameters of the algorithm
                int send_val = msgsnd(PG2S_msqid, &message, sizeof(message.algo)+sizeof(message.quantum)+sizeof(proc_cnt), !IPC_NOWAIT);

                if (send_val == -1)
                    perror("Process Generator: Errror in sending algo details to Scheduler");

                int sentProcesses = 0;

                while(1){
                    cur_clk= getClk();

                    if(cur_clk!=prev_clk && cur_clk!=0){
                        // send the count of processes that their arrival time =clk;
                        struct msgbuff m;
                        m.val=0;
                        if(proc_cnt==sentProcesses)
                            m.val=-1;
                        else{
                            for(int k=0;k<proc_cnt;k++){
                                if(processes[k].arrivalTime==cur_clk){
                                    m.val++;
                                }
                            }
                        }
                        m.mtype=sch_pid;
                        printf("PG:: m.val %d , clk %d\n",m.val,getClk());
                        int send_val = msgsnd(M_PG2S_msqid, &m, sizeof(m.val), !IPC_NOWAIT);
                        if (send_val == -1)
                            perror("Process Generator: Errror in sending process  to Scheduler");
                        // loop over the processes and send the process whose arrival time
                        // is equal to the current clk to the scheduler through the message queue
                        for(int k=0;k<proc_cnt;k++){
                            if(processes[k].arrivalTime==cur_clk){
                                struct msgProcess p;
                                p.p=processes[k];
                                p.mtype=sch_pid;
                                printf("\nPG :: The current time is %d\n",cur_clk);
                                printf("PG::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                                    ,p.p.id,p.p.arrivalTime,p.p.runTime,p.p.priority,p.p.memSize,p.p.pid);
                                send_val = msgsnd(PG2S_msqid, &p, sizeof(p.p), !IPC_NOWAIT);
                                if (send_val == -1)
                                    perror("Process Generator: Errror in sending process  to Scheduler");
                                sentProcesses+=1;
                            }
                        }
                        // printf("sentProcesssss %d,proc_Cnt %d clk %d\n",sentProcesses,proc_cnt,getClk());
                        prev_clk=cur_clk;

                        // Check if scheduler has finished then the process generator should terminate and clear the resources.
                        int status;
                        pid_t result=waitpid(sch_pid,&status,WNOHANG);
                        if(result!=0 && result!=-1){
                            printf("SCHEDULER FINISHEDDDDDD %d result %d\n",sch_pid,result);
                            destroyClk(true);
                            clearResources();
                            msgctl(PG2S_msqid, IPC_RMID, (struct msqid_ds *)0);
                            msgctl(M_PG2S_msqid, IPC_RMID, (struct msqid_ds *)0);
                            break;
                        }
                        

                    }////////


                }
               

            }




    }

}

//This function reads the processes txt file line by line
// and call another function "splitTokens" to update the information
// of the processes by the txt file'data.
void readProcessesData(char*file_name,struct process*processes){
    FILE *fp;
    char str[MAXCHAR];
 
    fp = fopen(file_name, "r");
    if (fp == NULL){
        printf("Could not open file %s",file_name);
        return;
    }
    int i=-1;
    while (fgets(str, MAXCHAR, fp) != NULL){
        // printf("%s", str);
        if(i!=-1) // Don't split tokens for first line in the file
            splitTokens(str,&processes[i]); // split string into tokens 1st Token ==> #id ,2ndToken==>arrivalTime and etc.
        i++;
    }
    fclose(fp);
}
// This function splits every line in processes.txt into tokens 
// where every token has an info about the process 1st token==> id , 2nd token ==> arrivaltIme and etc.
// and update the process's info by the splitted tokens form the file.
void splitTokens(char*str,struct process*p){

  char * pch;
//   printf ("Splitting string into tokens:\n");
  pch = strtok (str," ,.-");
  int j=0;
  while (pch != NULL)
  {
    int token=atoi(pch);
    if (j==0)
        p->id=token;
    else if(j==1)
        p->arrivalTime=token;
    else if (j==2){
        p->runTime=token;
        p->remainingTime=token;
    }
    else if(j==3)
        p->priority=token;
    else 
        p->memSize=token;
    pch = strtok (NULL, " ,.-");
    j++;
  }
  p->pid=-1;
  p->status=0;
  p->wait_time=0;
  p->lstfinish_time=0;
  p->startAlloc=-1;
  p->WQorRQ=0;
}

// This function counts the Number of processes that are read from the processes text file. 
void getProcessesCnt(char*file_name){
    FILE *fileptr;
    char chr;
    fileptr = fopen(file_name, "r");
    chr = getc(fileptr);

    while (chr != EOF){

        if (chr == '\n'){
            proc_cnt+=1;
        }
        chr = getc(fileptr);

    }

    fclose(fileptr); //close file.
}

void clearResources()
{
    //TODO Clears all resources in case of interruption
    printf("\nAbout to Destroy Message queue\n");
    msgctl(PG2S_msqid, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
