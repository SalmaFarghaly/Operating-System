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



//================Utilities for memory alloaction ====================////
// Initializing Memory 
bool A256[4]={0,0,0,0};
bool A128[8]={0,0,0,0,0,0,0,0};
bool A64[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A32[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A16[64]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A8[128]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A4[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A2[512]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool A1[1024]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool Allocation(struct process*p){
    int ProcessSize = p->memSize;

    if(ProcessSize<=256 && ProcessSize>128){ // Allocate in a 256 block
        for (int i =0; i<4; i++){
            if (A256[i]==0){ // then this block is free
                A256[i]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A128[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A64[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A32[w]=1;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A16[y]=1;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A8[f]=1;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A4[r]=1;
                }
                for(int e=i*128;e<i*128+128;e++){
                    A2[e]=1;
                }
                for(int t=i*256;t<i*256+256;t++){
                    A1[t]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*256,i*256+255);
            fflush(fp);
            fclose(fp);
            return true;
            }
        }
    }
    // process size smaller than 128
    if(ProcessSize>64 && ProcessSize<=128){
        for (int i =0; i<8; i++){
            if (A128[i]==0){ // then this block is free
                A128[i]=1;
                A256[i/2]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A64[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A32[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A16[w]=1;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A8[y]=1;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A4[f]=1;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A2[r]=1;
                }
                for(int e=i*128;e<i*128+128;e++){
                    A1[e]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*128,i*128+127);
            fflush(fp);
            fclose(fp);
                
                return true;
            }
        }
    }
    // process size smaller than 64
    if(ProcessSize>32 && ProcessSize<=64){
        for (int i =0; i<16; i++){
            if (A64[i]==0){ // then this block is free
                A64[i]=1;
                A128[i/2]=1;
                A256[i/4]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A32[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A16[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A8[w]=1;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A4[y]=1;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A2[f]=1;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A1[r]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*64,i*64+63);
            fflush(fp);
            fclose(fp);
              
                
                return true;
            }
        }
    }
    // process size smaller than 32 and bigger than 16
    if(ProcessSize>16 && ProcessSize<=32){
        for (int i =0; i<32; i++){
            if (A32[i]==0){ // then this block is free
                A32[i]=1;
                A64[i/2]=1;
                A128[i/4]=1;
                A256[i/8]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A16[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A8[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A4[w]=1;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A2[y]=1;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A1[f]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*32,i*32+31);
            fflush(fp);
            fclose(fp);
              
                return true;
            }
        }
    }
        // process size smaller than 16 and bigger than 8
    if(ProcessSize>8 && ProcessSize<=16){
        for (int i =0; i<64; i++){
            if (A16[i]==0){ // then this block is free
                A16[i]=1;
                A32[i/2]=1;
                A64[i/4]=1;
                A128[i/8]=1;
                A256[i/16]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A8[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A4[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A2[w]=1;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A1[y]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*16,i*16+15);
            fflush(fp);
            fclose(fp);
              
                return true;
            }
        }
    }
            // process size smaller than 16 and bigger than 8
    if(ProcessSize>4 && ProcessSize<=8){
        for (int i =0; i<128; i++){
            if (A8[i]==0){ // then this block is free
                A8[i]=1;
                A16[i/2]=1;
                A32[i/4]=1;
                A64[i/8]=1;
                A128[i/16]=1;
                A256[i/32]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A4[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A2[q]=1;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A1[w]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*8,i*8+7);
            fflush(fp);
            fclose(fp);
                return true;
            }
        }
    }
               // process size smaller than 16 and bigger than 8
    if(ProcessSize>2 && ProcessSize<=4){
        for (int i =0; i<256; i++){
            if (A4[i]==0){ // then this block is free
                A4[i]=1;
                A8[i/2]=1;
                A16[i/4]=1;
                A32[i/8]=1;
                A64[i/16]=1;
                A128[i/32]=1;
                A256[i/64]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A2[j]=1;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A1[q]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*4,i*4+3);
            fflush(fp);
            fclose(fp);
                return true;
            }
        }
    }
                   // process size smaller than 16 and bigger than 8
    if(ProcessSize>1 && ProcessSize<=2){
        for (int i =0; i<512; i++){
            if (A2[i]==0){ // then this block is free
                A2[i]=1;
                A4[i/2]=1;
                A8[i/4]=1;
                A16[i/8]=1;
                A32[i/16]=1;
                A64[i/32]=1;
                A128[i/64]=1;
                A256[i/128]=1;
                p->startAlloc=i;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A1[j]=1;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*2,i*2+1);
            fflush(fp);
            fclose(fp);
                return true;
            }
        }
    }
                       // process size smaller than 16 and bigger than 8
    if(ProcessSize<=1){
        for (int i =0; i<1024; i++){
            if (A1[i]==0){ // then this block is free
                A1[i]=1;
                A2[i/2]=1;
                A4[i/4]=1;
                A8[i/8]=1;
                A16[i/16]=1;
                A32[i/32]=1;
                A64[i/64]=1;
                A128[i/128]=1;
                A256[i/256]=1;
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i,i+1);
            fflush(fp);
            fclose(fp);
                return true;
                
            }
        }
    }
    // for(int i =0;i<64;i++){
    //     printf("hi %d--------------- A16[i] = %d\n",i,A16[i]);
    // }
    printf("REturnnnnnnn falseeeeeeeee\n");
    return false;
}
void De_Allocation(struct process*p){
    int ProcessSize = p->memSize;
    if(ProcessSize<=256 && ProcessSize>128){ // Allocate in a 256 block
        for (int i =0; i<4; i++){
            if (p->startAlloc==i){
                //Allocating all other memorry arrays
                A256[i]=0;
                
                for(int j=i*2;j<i*2+2;j++){
                    A128[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A64[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A32[w]=0;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A16[y]=0;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A8[f]=0;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A4[r]=0;
                }
                for(int e=i*128;e<i*128+128;e++){
                    A2[e]=0;
                }
                for(int t=i*256;t<i*256+256;t++){
                    A1[t]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*256,i*256+255);
            fflush(fp);
            fclose(fp);
               // return true;
               break;
            }
            }
        }
    
    // process size smaller than 128
    if(ProcessSize>64 && ProcessSize<=128){
        for (int i =0; i<8; i++){
            if (p->startAlloc==i){
            
                A128[i]=0;
                A256[i/2]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A64[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A32[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A16[w]=0;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A8[y]=0;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A4[f]=0;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A2[r]=0;
                }
                for(int e=i*128;e<i*128+128;e++){
                    A1[e]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*128,i*128+127);
            fflush(fp);
            fclose(fp);
                
                //return true;
                break;
            }
        }
    }
    
    // process size smaller than 64
    if(ProcessSize>32 && ProcessSize<=64){
        for (int i =0; i<16; i++){
            if (p->startAlloc==i){
                A64[i]=0;
                A128[i/2]=0;
                A256[i/4]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A32[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A16[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A8[w]=0;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A4[y]=0;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A2[f]=0;
                }
                for(int r=i*64;r<i*64+64;r++){
                    A1[r]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\freed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*64,i*64+63);
            fflush(fp);
            fclose(fp);
              
                break;
               // return true;
            }
        }
    }
    // process size smaller than 32 and bigger than 16
    if(ProcessSize>16 && ProcessSize<=32){
        for (int i =0; i<32; i++){
            if (p->startAlloc==i){
                A32[i]=0;
                A64[i/2]=0;
                A128[i/4]=0;
                A256[i/8]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A16[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A8[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A4[w]=0;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A2[y]=0;
                }
                for(int f=i*32;f<i*32+32;f++){
                    A1[f]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*32,i*32+31);
            fflush(fp);
            fclose(fp);
              break;
              //  return true;
            }
        }
    }
        // process size smaller than 16 and bigger than 8
    if(ProcessSize>8 && ProcessSize<=16){
        for (int i =0; i<64; i++){
            if (p->startAlloc==i){
                A16[i]=0;
                A32[i/2]=0;
                A64[i/4]=0;
                A128[i/8]=0;
                A256[i/16]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A8[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A4[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A2[w]=0;
                }
                for(int y=i*16;y<i*16+16;y++){
                    A1[y]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*16,i*16+15);
            fflush(fp);
            fclose(fp);
               break;
               // return true;
            }
        }
    }
            // process size smaller than 16 and bigger than 8
    if(ProcessSize>4 && ProcessSize<=8){
        for (int i =0; i<128; i++){
            if (p->startAlloc==i){
                A8[i]=0;
                A16[i/2]=0;
                A32[i/4]=0;
                A64[i/8]=0;
                A128[i/16]=0;
                A256[i/32]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A4[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A2[q]=0;
                }
                for(int w=i*8;w<i*8+8;w++){
                    A1[w]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*8,i*8+7);
            fflush(fp);
            fclose(fp);
               // return true;
               break;
            }
        }
    }
               // process size smaller than 16 and bigger than 8
    if(ProcessSize>2 && ProcessSize<=4){
        for (int i =0; i<256; i++){
            if (p->startAlloc==i){
                A4[i]=0;
                A8[i/2]=0;
                A16[i/4]=0;
                A32[i/8]=0;
                A64[i/16]=0;
                A128[i/32]=0;
                A256[i/64]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A2[j]=0;
                }
                for(int q=i*4;q<i*4+4;q++){
                    A1[q]=0;
                }
            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\tallocated\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*4,i*4+3);
            fflush(fp);
            fclose(fp);
                //return true;
                break;
            }
        }
    }
                   // process size smaller than 16 and bigger than 8
    if(ProcessSize>1 && ProcessSize<=2){
        for (int i =0; i<512; i++){
            if (p->startAlloc==i){
                A2[i]=0;
                A4[i/2]=0;
                A8[i/4]=0;
                A16[i/8]=0;
                A32[i/16]=0;
                A64[i/32]=0;
                A128[i/64]=0;
                A256[i/128]=0;
                //Allocating all other memorry arrays
                for(int j=i*2;j<i*2+2;j++){
                    A1[j]=0;
                }
            fprintf(fp, "At time\t%d\tfreed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i*2,i*2+1);
            fflush(fp);
            fclose(fp);
              //  return true;
              break;
            }
        }
    }
                       // process size smaller than 16 and bigger than 8
    if(ProcessSize<=1){
        for (int i =0; i<1024; i++){
            if (p->startAlloc==i){
                A1[i]=0;
                A1[i]=0;
                A2[i/2]=0;
                A4[i/4]=0;
                A8[i/8]=0;
                A16[i/16]=0;
                A32[i/32]=0;
                A64[i/64]=0;
                A128[i/128]=0;
                A256[i/256]=0;
                            fp = fopen("logs/memory/scheduler2.log", "a");
            fprintf(fp, "At time\t%d\freed\t%d\t\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),p->memSize,
            p->id,i,i+1);
            fflush(fp);
            fclose(fp);
                //return true;
                break;
                
            }
        }
    }
    // for(int i =0;i<64;i++){
    //     printf("hi %d--------------- A16[i] = %d\n",i,A16[i]);
    // }
   // return false;
}
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
    // printf("\nSechedular: I have just begun clkk %d\n",getClk());
    // fprintf(fp, "#At time\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\t\n");
    // fflush(fp);
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

struct process* getCurProcess(struct Queue*RR_Queue,struct Queue*Wait_Q,struct process *prev_process){
    printf("in GETTTTTT CURRRRRRRRRRRRRR clkkkkk %d\n",getClk());
    printf("in the wait queeeeeeueeeeeee \n");
    print_Queue(Wait_Q);
    printf("RRRRRRR queeeeeeueeeeeee \n");
    print_Queue(RR_Queue);
    struct process* tempcur_process=NULL;
    // dequeue(RR_Queue);
    // if the wait Queue is not empty listen to it
    // and find the first one that can fit the memory
    if(isEmpty(Wait_Q)==false){
        struct Node*start=Wait_Q->headPtr;
        while(Allocation(start->processObj)==false){
            start=start->nextNodePtr;
            //cant find any one to allocate inside wait Queue
            // look at the ready Queue
            if(start==NULL){
                //dequeue a node from read queue 
                // if it was allocated before then schedule it
                // if not check if it can be allocated
                while(true){
                    struct process* R_start=dequeue(RR_Queue);
                    if(R_start == NULL){
                        // enqueue(RR_Queue,prev_process);
                        return prev_process;
                    }
                    if(R_start->startAlloc!=-1){
                        tempcur_process=R_start;
                        // enqueue(RR_Queue,tempcur_process);
                        return tempcur_process;
                    }
                    if(Allocation(R_start)==true){
                        printf("alloacteeee clkkkk %d iddddd %d\n",getClk(),R_start->id);
                        tempcur_process=R_start;
                        // enqueue(RR_Queue,tempcur_process);
                        return tempcur_process;
                    }
                    //if the process that in Ready queue cant be allocated and wasnt 
                    //scheduled before put it in wait queue.
                    else{
                        printf("%d %d %d %d \n",A256[0],A256[1],A256[2],A256[3]);
                        printf("can't alloacteeee clkkkk %d iddddd %d\n",getClk(),R_start->id);
                        enqueue(Wait_Q,R_start);
                    }
                }
            }
            
        }
        // if you found a process in wait_Queue that can be allocated
        // schedule it and remove it from wait_Queue
        // and enqueue it to ready queue
        tempcur_process=start->processObj;
        enqueue(RR_Queue,tempcur_process);
        remove_Node(Wait_Q,tempcur_process);
        return tempcur_process;

    }
    else{
        // if the wait queue is empty then choose the first 
        // this block is the same as block from line 747 till line 768.
        while(true){
            struct process* R_start=dequeue(RR_Queue);
            if(R_start == NULL){
                    // enqueue(RR_Queue,prev_process);
                    return prev_process;
                }
                if(R_start->startAlloc!=-1){
                    tempcur_process=R_start;
                    // enqueue(RR_Queue,tempcur_process);
                    return tempcur_process;
                }
                if(Allocation(R_start)==true){
                    printf("alloacteeee clkkkk %d iddddd %d\n",getClk(),R_start->id);
                    tempcur_process=R_start;
                    // enqueue(RR_Queue,tempcur_process);
                    return tempcur_process;
                }
                else{
                      printf("%d %d %d %d \n",A256[0],A256[1],A256[2],A256[3]);
                    printf("can't alloacteeee clkkkk %d iddddd %d\n",getClk(),R_start->id);
                    enqueue(Wait_Q,R_start);
            }
        }

    }
    return tempcur_process;

}

void RoundRobin(){

    
    sch=getpid();
    struct Queue*RR_Queue=Queue_Constructor();
    struct Queue *Wait_Q=Queue_Constructor();
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

                        // cur_process->status=-1;
                    }

                }
            }
            if(handler==1){
                printf("AMMMMMM IN HANDLLERRRRRRRR %d\n",getClk());

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
                De_Allocation(cur_process);
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
        if(cur_process!=NULL){
            if(RR_allFinished(RR_Queue)&&cur_process->status==-1&&rec==-1){
                printf("SCH::CLK   %d  EXITING\n",getClk());
                return;
            }
        }
        //if there is process that should be stopped;
        //-1 cur has finished or number pid to stop it

        if(cur_process!=NULL){
            if(cur_process->status!=1){ // stopped 0 finished -1 not started yet 0

                struct process*prev_process=cur_process; //make previous process the current one that we should stop at this clk cycle
                cur_process=getCurProcess(RR_Queue,Wait_Q,prev_process); // get a new process to run.
                printf("PROCESSSSSSSSSS %d is chosennnnnn clk  %d\n",cur_process->id,getClk());
                // cur_process=peekFront(RR_Queue);
                // if(cur_process==NULL){
                //     enqueue(RR_Queue,prev_process);
                //     cur_process=prev_process;
                // }
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