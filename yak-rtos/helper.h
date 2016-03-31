//------------------------------------------------------------------------
// YAK Helper Function Header File
//  - Description: Data used by YAK kernel helper functions
//------------------------------------------------------------------------

// Global Data (defined in yak_c.c)
extern int YKCtxSwCount;
extern int YKIdleCount; 
extern int YKTickNum; 
extern int YKInterruptNestLevel;
extern char YKStartedFlag;
extern YKSEM YKSemaphores[];
extern YKQ YKQueues[];
extern int YKIdleStack[];
extern TCB YKTasks[];
extern TCBPtr YKCurrentTask; 
extern TCBPtr YKReadyHead;
extern TCBPtr YKReadyTail;
extern TCBPtr YKDelayedHead;
extern TCBPtr YKDelayedTail; 

// Debug Commands
typedef enum {READY, DELAYED, CURRENT, DUMP} DCMD;

// Helper C Function Prototypes
void insertReady(TCBPtr);
void removeReady(TCBPtr);
void insertDelayed(TCBPtr);
TCBPtr removeDelayed();
void insertPendSem(TCBPtr, YKSEM*);
void insertPendQ(TCBPtr, YKQ*);
TCBPtr removePendSem(YKSEM*);
TCBPtr removePendQ(YKQ*);
TCBPtr allocateTCB();
YKSEM* allocateSemaphore();
YKQ* allocateQueue();
void debug(DCMD);
void printTask(TCBPtr);
