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
char myname[19]="Process Generator: ";
int  proc_cnt=0;  // number of processes
int cur_clk=-1;
int prev_clk=-1;



void clearResources(int);
void readProcessesData(char*file_name,struct process*processes);
void splitTokens(char*str,struct process*pro);
void getProcessesCnt();

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // 1. Read the input files.
    char*file_name="testcases/processes1.txt";
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
    printf("Enter your choice : ");

    scanf("%d", &choice);
    switch (choice){
        case 1:
            printf("\nEnter the quantum: ");
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
        printf("I am the child my pid : %d\n",getpid());
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
                char*args[]={"./scheduler.out",NULL};
                execv(args[0],args);
                exit(0);
            }
            else{
                key_t PG2S_key_id;
            

                // Create a message Queue to send info to scheduler.
                PG2S_key_id= ftok("keyfile", 1);
                PG2S_msqid = msgget(PG2S_key_id, IPC_CREAT | 0644);
                if (PG2S_msqid == -1){
                    printf("%s",myname);
                    perror("Error in create message queue to Scheduler\n");
                    exit(-1);
                }
                printf("message Queue:%d\n",PG2S_msqid);


                // Send which Algo to run as a message through queue to the scheduler
                struct algoMsgBuff  message;
                message.algo=choice;
                message.quantum=quantum;
                message.mtype=sch_pid%10000;

                // Send message to the scheduler contains which algorithm to run and the parameters of the algorithm
                int send_val = msgsnd(PG2S_msqid, &message, sizeof(message.algo)+sizeof(message.quantum), !IPC_NOWAIT);

                if (send_val == -1)
                    perror("Process Generator: Errror in sending algo details to Scheduler");

                int sentProcesses = 0;
                int temp_process=0;
                // sleep(1);

                while(1){
                    cur_clk= getClk();

                    if(cur_clk!=prev_clk&&cur_clk!=0){
                        // loop over the processes and send the process whose arrival time
                        // is equal to the current clk to the scheduler through the message queue
                        struct msgbuff m;
                        m.val=0;
                        for(int k=0;k<proc_cnt;k++){
                            if(processes[k].arrivalTime==cur_clk){
                                m.val++;
                            }
                        }
                        m.mtype=sch_pid;
                        printf("PG:: m.val %d , clk %d\n",m.val,getClk());
                        int send_val = msgsnd(PG2S_msqid, &m, sizeof(m.val), !IPC_NOWAIT);
                        if (send_val == -1)
                            perror("Process Generator: Errror in sending process  to Scheduler");
                        for(int k=0;k<proc_cnt;k++){
                            if(processes[k].arrivalTime==cur_clk){
                                struct msgProcess p;
                                p.p=processes[k];
                                p.mtype=sch_pid;
                                printf("\nPG :: The current time is %d\n",cur_clk);
                                send_val = msgsnd(PG2S_msqid, &p, sizeof(p.p), !IPC_NOWAIT);
                                if (send_val == -1)
                                    perror("Process Generator: Errror in sending process  to Scheduler");
                                sentProcesses+=1;
                                temp_process++;
                            }
                        }
                        // printf("Proccc %d snet%d\n",proc_cnt,sentProcesses);
                        // if(temp_process==0){
                           
                        //     struct msgbuff m;
                        //     m.val=0;
                        //     m.mtype=sch_pid;
                        //     printf("PG:: m.val %d , clk %d\n",m.val,getClk());
                        //     int send_val = msgsnd(PG2S_msqid, &m, sizeof(m.val), !IPC_NOWAIT);
                        //     if (send_val == -1)
                        //     perror("Process Generator: Errror in sending process  to Scheduler");
                        // }
                        if (sentProcesses == proc_cnt){
                                printf("\n");
                                struct msgbuff m;
                            m.val=-1;
                            m.mtype=sch_pid;
                            printf("PG:: m.val %d , clk %d\n",m.val,getClk());
                            int send_val = msgsnd(PG2S_msqid, &m, sizeof(m.val), !IPC_NOWAIT);
                            if (send_val == -1)
                            perror("Process Generator: Errror in sending process  to Scheduler");
                                // printf("halloooooooooooooooooooooooooooooooo");
                                //sleep 1 second before termination so the scheduler can read the last message
                                sleep(2);
                                printf("\nDone Sending Processes, About to Destroy Message queue\n");
                                msgctl(PG2S_msqid, IPC_RMID, (struct msqid_ds *)0);
                                break;
                        }
                        prev_clk=cur_clk;
                        temp_process=0;
                        


                    }


                }
                destroyClk(true);

            }
            // TODO Generation Main Loop
            // 5. Create a data structure for processes and provide it with its parameters.

            // 6. Send the information to the scheduler at the appropriate time.
            // 7. Clear clock resources




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
        printf("%s", str);
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
  printf ("Splitting string into tokens:\n");
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

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nAbout to Destroy Message queue\n");
    msgctl(PG2S_msqid, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
