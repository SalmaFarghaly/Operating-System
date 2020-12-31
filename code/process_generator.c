#include "headers.h"
#include "string.h"
#include "stdlib.h"
#define MAXCHAR 1000


void clearResources(int);
void readProcessesData();
void splitTokens();

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    char*file_name="testcases/processes1.txt";
    readProcessesData(file_name);
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void readProcessesData(char*file_name){
    FILE *fp;
    char str[MAXCHAR];

 
    fp = fopen(file_name, "r");
    if (fp == NULL){
        printf("Could not open file %s",file_name);
        return;
    }
    int i=0;
    while (fgets(str, MAXCHAR, fp) != NULL){
        printf("%s", str);
        if(i!=0) // Don't split tokens for first line in the file
            splitTokens(str); // split string into tokens 1st Token ==> #id ,2ndToken==>arrivalTime and etc.
        i=1;
    }
    fclose(fp);
}

void splitTokens(char*str){

  char * pch;
  printf ("Splitting string into tokens:\n");
  pch = strtok (str," ,.-");
  while (pch != NULL)
  {
    int token=atoi(pch);
    printf ("The number %d\n",token);
    pch = strtok (NULL, " ,.-");
  }
  printf("\n");
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
