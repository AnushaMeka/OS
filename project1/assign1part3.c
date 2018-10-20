#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>
#include <math.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>


int pageFaults=0;
char *sequenceFileName;

// A Queue Node (Queue is implemented using Doubly Linked List)
typedef struct QNode
{   
    struct QNode *prev, *next; 
    unsigned pageNumber;  // the page number stored in this QNode
} QNode;

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

// A utility function to create an empty Hash of given capacity
Hash* createHash( int capacity )
{   
    // Allocate memory for hash 
    Hash* hash = (Hash *) malloc(sizeof(Hash));
    hash->capacity = capacity;
    
    // Create an array of pointers for refering queue nodes 
    hash->array = (QNode **) malloc(hash->capacity * sizeof(QNode*));
    
    // Initialize all hash entries as empty
    int i; 
    for( i = 0; i < hash->capacity; ++i )
        hash->array[i] = NULL;
    
    return hash;
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


void ReferencePage( Queue* queue, Hash* hash, unsigned long pageNumber )
{   
    QNode* reqPage = hash->array[ pageNumber ];
    // the page is not in cache, bring it
    if ( reqPage == NULL ) 
        Enqueue( queue, hash, pageNumber );
    
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

int search(Queue* queue, int long data, int num_of_frames) {
    int pos=0;
    int arr[num_of_frames];
    int i=0;
    struct QNode* temp = queue->front;
    if(temp == NULL) return 0;
    while(temp->next != NULL) {
        temp = temp->next;
    }
    while(temp != NULL) {
        //printf("data in queue is %d\n",temp->pageNumber);
        arr[pos] = temp->pageNumber;
        temp = temp->prev;
        pos++;
    }
    for(i=0;i<num_of_frames;i++){
       if(data == arr[i]){
          //printf("found at %d location\n",i+1);
          return i+1;
       }
    }
    return 0;
}

void getPage(Queue* q, Hash* hash, unsigned long logical_address, int pa_bytes, int page_size,  unsigned long offset_result, FILE **fileIO){
    int a,num_of_frames;
    unsigned long pageNum, physical_address;
    pageNum = logical_address/page_size;
    num_of_frames = pa_bytes/page_size;
    ReferencePage( q, hash, pageNum);
    a = search(q,pageNum,num_of_frames);
    physical_address = a*page_size+offset_result;
    char *returned_hexa = decToHexa(physical_address);
    fprintf(*fileIO," %s \n",returned_hexa);
    //FILE *output = fopen("output-part3","a");
    //fprintf(output," %s \n",returned_hexa);
    //fclose(output);
}

size_t alignup(size_t size, size_t alignto) {
  return ((size + (alignto - 1)) & ~(alignto - 1));
}

void analyzeAccessSequenceFromFile(char * fileName, int page_size, int va_bytes, int pa_bytes) {        
	// Open this file
        int fd, i;
        struct stat st;
        unsigned long filesize, mapsize;
        unsigned long logical_address, offset_result;
        char buffer[1000];
        unsigned long * memAccesses;
        //zeroth frame for OS
	
	Queue* q = createQueue(((pa_bytes/page_size)-1));
	Hash* hash = createHash(((va_bytes/page_size)));
	stat(fileName, &st);
	filesize = st.st_size;

        fd = open(fileName, O_RDONLY);
        if(fd == -1) {
                fprintf(stderr, "fd is invalid, with error %s\n", strerror(errno));
                exit(-1);
        }

        // Compute the aligned size;
        mapsize = alignup (filesize, va_bytes);
        memAccesses = (unsigned long *)mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fd, 0);
        if(memAccesses == MAP_FAILED) {
                fprintf(stderr, "mmap the input file failed. \n");
                exit(-1);
        }
        FILE *output = fopen("output-part3","w+");
	fprintf(stderr, "map starting %p filesize %ld\n", memAccesses, filesize);
        for(i = 0; i < filesize/sizeof(unsigned long); i++) {
                //printf("%d: %lx\n", i, memAccesses[i]);
                sprintf(buffer, "%lx", memAccesses[i]);
                logical_address = hexadecimalToDecimal(buffer);
                offset_result = logical_address%page_size;
                // get the physical address
                getPage(q, hash, logical_address, pa_bytes, page_size, offset_result, &output);
        }

        //fclose(sequenceFileName);
}

int main(int argc, char *argv[])
{
    int  page_size, va_bytes, pa_bytes;

    // perform basic error checking
    if (argc != 5) {
        fprintf(stderr,"Usage: ./a.out [page/frame size] [virtual memory size] [physical memory size] [input sequence file]\n");
    }
    sequenceFileName = argv[4];
    //converting command line arguments from string to integer
    page_size = atoi(argv[1]);
    va_bytes = atoi(argv[2]);
    pa_bytes = atoi(argv[3]);
    // Check whether the sequence file is existing and I can access this file?
    if(access(sequenceFileName, F_OK ) == -1 ) {
        fprintf(stderr, "The sequence file %s is not existing\n", sequenceFileName);
        exit(-1);
    }

    // perform basic error checking
    analyzeAccessSequenceFromFile(sequenceFileName, page_size, va_bytes, pa_bytes);
    printf("page faults : %d\n",pageFaults);
    return 0;
}


