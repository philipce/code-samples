//------------------------------------------------------------------------
// YAK Kernel Header File
//  - Description: Data used by YAK kernel
//------------------------------------------------------------------------

// Kernel Constants
#define NULL 0
#define DEFAULT_FLAGS 0x0200
#define IDLE_STACK_SIZE 256
#define IDLE_TASK_ID 0
#define CONTEXT_SIZE 13
#define DELAY_INTERRUPT_LENGTH 5000

// Global Data (defined in yak_c.c)
extern int YKCtxSwCount;
extern int YKIdleCount; 
extern int YKTickNum; 

// Task Control Block Data Structure
typedef struct taskStruct *TCBPtr;
typedef struct taskStruct
{
    int *sp;
    int *stackBottom; // for debug purposes only
    int taskID;
    int delayCount;
    unsigned char priority;
    TCBPtr next; // can link ready, delayed, or pending tasks
    TCBPtr prev;
} TCB;

// Semaphore Data Structure
typedef struct semaphoreStruct
{
    int value;
    TCBPtr pendHead; // head of the list of tasks pending on this semaphore
} YKSEM;

// Queue Data Structure
typedef struct queueStruct
{
    unsigned numEntries;
    unsigned maxEntries;
    void **start; // first slot in queue's array
    void **end; // last slot in queue's array
    void **nextSlot; // next empty slot to insert to
    void **nextMsg; // next message to remove
    TCBPtr pendHead;  // head of the list of tasks pending on a msg from this queue
} YKQ;

// YAK Kernel Macros
YKNoOp();
#define YKNoOp() asm("nop")

// YAK Kernel C Function Prototypes
void YKInitialize(void);
void YKIdleTask(void);
void YKNewTask(void (*)(void), void *, unsigned char);
void YKRun(void);
void YKScheduler();
void YKEnterISR(void);
void YKExitISR(void);
void YKDelayTask(unsigned);
YKSEM* YKSemCreate(int);
void YKSemPend(YKSEM*);
void YKSemPost(YKSEM*);
YKQ* YKQCreate(void **, unsigned);
void* YKQPend(YKQ*);
int YKQPost(YKQ *, void *);

// YAK Kernel Assembly Function Prototypes
void YKDispatcher();
int YKEnterMutex();
void YKExitMutex();
