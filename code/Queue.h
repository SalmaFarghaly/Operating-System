#include "headers.h"

struct Node;

struct Node{
    struct process* processObj;
    struct Node* nextNodePtr;
};

struct Queue{
    struct Node* headPtr;
    struct Node* tailPtr;
};

//Creates a Node
struct Node* Node_Constructor(struct process* processObj){
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->processObj = processObj;
    newNode->nextNodePtr = NULL;
    return newNode;
}

//Creates an empty queue
struct Queue* Queue_Constructor(){
    struct Queue* newQ = (struct Queue*)malloc(sizeof(struct Queue));
    newQ->headPtr = NULL;
    newQ->tailPtr = NULL;
    return newQ;
}

//Frees memory allocated by the node constructor
void Node_Destructor(struct Node* n){
    free(n);
}

//Checks if queue is empty
bool isEmpty(struct Queue* q){
    if(q->headPtr==NULL){
        return true;
    }else{
        return false;
    }
}

//Add the process to the end of the queue
void enqueue(struct Queue* q , struct process* p){
    struct Node* processNode = Node_Constructor(p);
    if(isEmpty(q)){
        q->headPtr = processNode;
        q->tailPtr = processNode;
        return;
    }
    q->tailPtr->nextNodePtr = processNode;
    q->tailPtr = processNode;
    return;
}

//Removes the process at the start of the queue and returns it
//returns null if the queue is empty
struct process* dequeue(struct Queue* q){
    if(isEmpty(q)){
        return NULL;
    }
    struct Node * tempNode = q->headPtr;
    q->headPtr = q->headPtr->nextNodePtr;
    if(q->headPtr==NULL){
        q->tailPtr = NULL ;
    }
    struct process* tempProcess = tempNode->processObj;
    Node_Destructor(tempNode);
    return tempProcess;
}

//return the process at the start of the queue without removing it from the queue
struct process* peekFront(struct Queue* q){
    if(isEmpty(q)){
        return NULL;
    }
    return q->headPtr->processObj;
}

//Frees the memory allocated by the Queue constructor
void Queue_Destructor(struct Queue* q){
    while(!isEmpty(q)){
        dequeue(q);
    }
    free(q);
}

// void print_Queue(struct Queue*q){
//     struct Node*cur_ptr=q->headPtr;
//     while(cur_ptr!=NULL){
//         printf(cur_ptr->processObj);
//         cur_ptr=cur_ptr->nextNodePtr;
//     }
// }