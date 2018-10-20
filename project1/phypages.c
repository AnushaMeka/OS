#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>
#include <math.h>
#include "assign1part2.h"
//int pageFaults = 0;
struct QNode
{
    struct QNode *prev, *next;
    unsigned pageNumber;  // the page number stored in this QNode
};

// A Queue (A FIFO collection of Queue Nodes)
typedef struct Queue
{
    unsigned count;  // Number of filled frames
    unsigned numberOfFrames; // total number of frames
    QNode *front, *rear, *current;
} Queue;

// A hash (Collection of pointers to Queue Nodes)
typedef struct Hash
{
    int capacity; // how many pages can be there
    QNode* *array; // an array of queue nodes
} Hash;

// A utility function to create a new Queue Node. The queue Node
// will store the given 'pageNumber'
QNode* newQNode( unsigned pageNumber )
{
    // Allocate memory and assign 'pageNumber'
    QNode* temp = (QNode *)malloc( sizeof( QNode ) );
    temp->pageNumber = pageNumber;

    // Initialize prev and next as NULL
    temp->prev = temp->next = NULL;

    return temp;
}

// A utility function to create an empty Queue.
// The queue can have at most 'numberOfFrames' nodes
Queue* createQueue( int numberOfFrames )
{
    Queue* queue = (Queue *)malloc( sizeof( Queue ) );

    // The queue is empty
    queue->count = 0;
    queue->front = queue->rear = queue->current = NULL;

    // Number of frames that can be stored in memory
    queue->numberOfFrames = numberOfFrames;

    return queue;
}

// A function to check if there is slot available in memory
int AreAllFramesFull( Queue* queue )
{
    return queue->count == queue->numberOfFrames;
}

// A utility function to check if queue is empty
int isQueueEmpty( Queue* queue )
{
    return queue->rear == NULL;
}

// A utility function to delete a frame from queue
void deQueue(Queue* queue)
{   
    if(isQueueEmpty(queue))
        return;
    
    // If this is the only node in list, then change front
    if (queue->front == queue->rear)
        queue->front = NULL;
    
    // Change rear and remove the previous rear
    QNode* temp = queue->rear; 
    queue->rear = queue->rear->prev;
    
    if (queue->rear) 
        queue->rear->next = NULL;
    
    free(temp);
    
    // decrement the number of full frames by 1
    queue->count--;
}

int search(Queue* queue, int data) {
        int pos=0;
        int arr[7];
        int i=0;
        struct QNode* temp = queue->front;
        if(temp == NULL) return 0; // empty list, exit
        // Going to last Node
        while(temp->next != NULL) {
                temp = temp->next;
        }
        // Traversing backward using prev pointer
        //printf("Reverse: ");
        while(temp != NULL) {
                //printf("data in queue is %d\n",temp->pageNumber);
                arr[pos] = temp->pageNumber;
                temp = temp->prev;
                pos++;
        }
        //printf("\n");
        for(i=0;i<=7;i++){
                if(data == arr[i]){
                   //printf("found at %d location\n",i+1);
                   return i+1;
                }
        }      
        return 0;
}
