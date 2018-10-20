
/*typedef struct QNode
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
} Hash;*/

typedef struct QNode QNode;
typedef struct Queue Queue;
typedef struct Hash Hash;
unsigned long hexadecimalToDecimal(char hexVal[]);
char * decToHexa(unsigned long n);
QNode* newQNode( unsigned pageNumber );
Queue* createQueue( int numberOfFrames );
Hash* createHash( int capacity );
int AreAllFramesFull( Queue* queue );
void deQueue(Queue* queue);
void Enqueue(Queue* queue, Hash* hash, unsigned pageNumber);
void ReferencePage( Queue* queue, Hash* hash, unsigned pageNumber);
int search(Queue* queue, int data);
void getPage(Queue* q, Hash* hash, unsigned long logical_address, unsigned long offset_result, FILE **fileIO);
size_t alignup(size_t size, size_t alignto);
void analyzeAccessSequenceFromFile(char * fileName);

