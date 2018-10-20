#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>
#include <math.h>
#include <math.h>
#include <sys/stat.h>
#include "assign1part2.h"

#define REAL_PAGE_SIZE 4096
#define PAGE_SIZE 128

int pageFaults=0;
char *sequenceFileName;

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

unsigned long hexadecimalToDecimal(char hexVal[])
{
    int len = strlen(hexVal);
    int base = 1;
    int dec_val = 0;
    for (int i=len-1; i>=0; i--) {
        if (hexVal[i]>='0' && hexVal[i]<='9') {
            dec_val += (hexVal[i] - 48)*base;
            base = base * 16;
        }
        else if (hexVal[i]>='A' && hexVal[i]<='F') {
            dec_val += (hexVal[i] - 55)*base;
            base = base*16;
        }
        else if (hexVal[i]>='a' && hexVal[i]<='f') {
            dec_val += (hexVal[i] - 87)*base;
            base = base*16;
        }
    }

    return dec_val;
}

char * decToHexa(unsigned long n)
{
    char hexaDeciNum[100];
    char *hexa = (char *) malloc(sizeof(char) * 16);
    int i = 0;
    int k = 0;
    while(n!=0) {
        int temp  = 0;
        temp = n % 16;
        if(temp < 10) {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else {
            hexaDeciNum[i] = temp + 87;
            i++;
        }
        n = n/16;
    }
    for(int j=i-1; j>=0; j--) {
        hexa[k] = hexaDeciNum[j];
        k++;
    }
    return hexa;
}

// A function to add a page with given 'pageNumber' to both queue
// and hash
void Enqueue(Queue* queue, Hash* hash, unsigned pageNumber)
{
    // If all frames are full, remove the page at the rear
    if (AreAllFramesFull(queue))
    {
        // remove page from hash
        pageFaults++;
        hash->array[ queue->rear->pageNumber ] = NULL;
        deQueue( queue );
    }

    // Create a new node with given page number,
    // And add the new node to the front of queue
    QNode* temp = newQNode( pageNumber );
    temp->next = queue->front;

    // If queue is empty, change both front and rear pointers
    if ( isQueueEmpty( queue ) )
        queue->rear = queue->front = temp;
    else  // Else change the front
    {
        queue->front->prev = temp;
        queue->front = temp;
    }

    // Add page entry to hash also
    hash->array[ pageNumber ] = temp;

    // increment number of full frames
    queue->count++;
}

void ReferencePage( Queue* queue, Hash* hash, unsigned pageNumber)
{   
    QNode* reqPage = hash->array[ pageNumber ];    
    // the page is not in cache, bring it
    if ( reqPage == NULL )
        Enqueue( queue, hash, pageNumber);
    
    // page is there and not at front, change pointer
    else if (reqPage != queue->front)
    {   
        // Unlink rquested page from its current location
        // in queue.
        reqPage->prev->next = reqPage->next;
        if (reqPage->next)
           reqPage->next->prev = reqPage->prev;
        
        // If the requested page is rear, then change rear
        // as this node will be moved to front
        if (reqPage == queue->rear)
        {
           queue->rear = reqPage->prev;
           queue->rear->next = NULL;
        }

        // Put the requested page before current front
        reqPage->next = queue->front;
        reqPage->prev = NULL;

        // Change prev of current front
        reqPage->next->prev = reqPage;

        // Change front to the requested page
        queue->front = reqPage;
    }

}

void getPage(Queue* q, Hash* hash, unsigned long logical_address, unsigned long offset_result, FILE **fileIO){
    //int pageFaults=0
    int a;
    unsigned pageNum;
    unsigned long physical_address;
    pageNum = logical_address/PAGE_SIZE;
    ReferencePage( q, hash, pageNum);
    a = search(q,pageNum);
    physical_address = a*PAGE_SIZE+offset_result;
    char *returned_hexa = decToHexa(physical_address);
    fprintf(*fileIO," %s \n",returned_hexa);    
    //FILE *output = fopen("output-part2","a");
    //fprintf(output," %s \n",returned_hexa);
    //fclose(output);
}

size_t alignup(size_t size, size_t alignto) {
  return ((size + (alignto - 1)) & ~(alignto - 1));
}

void analyzeAccessSequenceFromFile(char * fileName) {
        // Open this file
        int fd, i;
        struct stat st;
        unsigned long filesize, mapsize;
        unsigned long logical_address, offset_result;
        char buffer[100];
        unsigned long * memAccesses;
        // Let cache can hold 7 pages 1 for OS
        Queue* q = createQueue( 7 );
        // Let 32 different pages can be requested (pages to be
        // referenced are numbered from 0 to 31
        Hash* hash = createHash( 32 );
        stat(fileName, &st);

        filesize = st.st_size;

        fd = open(fileName, O_RDONLY);
        if(fd == -1) {
                fprintf(stderr, "fd is invalid, with error %s\n", strerror(errno));
                exit(-1);
        }

        // Compute the aligned size;
        mapsize = alignup (filesize, REAL_PAGE_SIZE);
        memAccesses = (unsigned long *)mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fd, 0);
        if(memAccesses == MAP_FAILED) {
                fprintf(stderr, "mmap the input file failed. \n");
                exit(-1);
        }
        FILE *output = fopen("output-part2","w+");
        fprintf(stderr, "map starting %p filesize %ld\n", memAccesses, filesize);
        for(i = 0; i < filesize/sizeof(unsigned long); i++) {
                printf("%d: %lx\n", i, memAccesses[i]);
                sprintf(buffer, "%lx", memAccesses[i]);
                logical_address = hexadecimalToDecimal(buffer);
                offset_result = logical_address%PAGE_SIZE;
                // get the physical address
                getPage(q,hash,logical_address, offset_result, &output);
        }

        //fclose(sequenceFileName);
}

int main(int argc, char *argv[])
{
    sequenceFileName = argv[1];
    //int pageFaults=0;
    // Check whether the sequence file is existing and I can access this file?
    if(access(sequenceFileName, F_OK ) == -1 ) {
        fprintf(stderr, "The sequence file %s is not existing\n", sequenceFileName);
        exit(-1);
    }

    // perform basic error checking
    if (argc != 2) {
        fprintf(stderr,"Usage: ./a.out [input file]\n");
        return -1;
    }
    analyzeAccessSequenceFromFile(sequenceFileName);
    printf("page faults : %d\n",pageFaults);
    return 0;
}
