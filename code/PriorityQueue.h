#include "headers.h"

typedef struct node { 
    struct process processobj; 
  
    // Lower values indicate higher priority 
    int priority; 
  
    struct node* next; 
  
} Node; 
  
Node* newNode( struct process proc , int p) 
{ 
    Node* temp = (Node*)malloc(sizeof(Node)); 
    temp->processobj = proc; 
    temp->priority = p; 
    temp->next = NULL; 
  
    return temp; 
} 
  
// Return the value at head 
struct process peek(Node** head) 
{ 
    return (*head)->processobj; 
} 
  
// Removes the element with the 
// highest priority form the list 
void pop(Node** head) 
{ 
    Node* temp = *head; 
    (*head) = (*head)->next; 
    free(temp); 
} 
  
// Function to push according to priority 
void push(Node** head, struct process proc , int p) 
{ 
    Node* start = (*head); 
  
    // Create new Node 
    Node* temp = newNode( proc, p); 
  
    // Special Case: The head of list has lesser 
    // priority than new node. So insert new 
    // node before head node and change head node. 
    if ((*head)->priority > p) { 
  
        // Insert New Node before head 
        temp->next = *head; 
        (*head) = temp; 
    } 
    else { 
  
        // Traverse the list and find a 
        // position to insert new node 
        while (start->next != NULL && 
               start->next->priority < p) { 
            start = start->next; 
        } 
  
        // Either at the ends of the list 
        // or at required position 
        temp->next = start->next; 
        start->next = temp; 
    } 
} 
  
// Function to check is list is empty 
int isEmpty(Node** head) 
{ 
    return (*head) == NULL; 
}


