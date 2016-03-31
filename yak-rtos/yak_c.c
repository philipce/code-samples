//------------------------------------------------------------------------
// YAK Kernel Code (C)
//  - Description: C library of YAK kernel functions
//------------------------------------------------------------------------

#include "clib.h"
#include "kernel.h"
#include "helper.h"
#include "user.h"

//--------------------------------
// GLOBAL VARIABLES
//--------------------------------

// Counters
int YKCtxSwCount;           // number of context switches
int YKIdleCount;            // used by idle task
int YKTickNum;              // incremented by tick handler
int YKInterruptNestLevel;

// Flags
char YKStartedFlag;

// Semaphores
YKSEM YKSemaphores[MAX_SEMAPHORES]; // array to allocate semaphores

// Queues
YKQ YKQueues[MAX_QUEUES]; // array to allocate queues

// Task Management
int YKIdleStack[IDLE_STACK_SIZE];   // idle task stack
TCB YKTasks[MAX_TASKS+1];           // array to allocate TCBs (+1 is for idle task)
TCBPtr YKCurrentTask;               // currently running task
TCBPtr YKReadyHead;                 // contains ready tasks
TCBPtr YKReadyTail;
TCBPtr YKDelayedHead;               // contains delayed tasks
TCBPtr YKDelayedTail; 

//--------------------------------
// KERNEL FUNCTIONS
//--------------------------------
void YKInitialize()
{
    // Initialize RTOS
    // - Must be called EXACTLY once before calling YKRun
    // - Dummy TCB created--task will never run, just ensures that dispatcher will be called on 
    //      first time scheduling

    static TCBPtr dummyTask;

    // Disable interrupts (on by default)
    YKEnterMutex();    

    // Create idle task so that there is always at least one ready task (per YAK specifications)
    YKNewTask(&YKIdleTask, &YKIdleStack[IDLE_STACK_SIZE-1] , LOWEST_TASK_PRIORITY);

    // Create dummy TCB
    dummyTask->sp = NULL;
    dummyTask->stackBottom = NULL; 
    dummyTask->taskID = -1;
    dummyTask->delayCount = 0;
    dummyTask->priority = LOWEST_TASK_PRIORITY;
    dummyTask->next = NULL;
    dummyTask->prev = NULL;  

    // Initialize YKCurrentTask
    YKCurrentTask = dummyTask;    

    return;
}

void YKIdleTask()
{
    // Spin in a loop incrementing a counter
    //  - Lowest priority task code
    //  - YKIdleCount used to determine CPU utilization (once every 20 ticks)
    //  - Loop MUST take 4 instructions/iteration to prevent overflow

    while(1)
    {
        asm("cli");
        YKIdleCount++;
        asm("nop");
        asm("sti");
    }
    return;
}

void YKNewTask(void (* taskCode)(void), void *taskStack, unsigned char taskPriority)
{
    // Create a new task for the RTOS
    //  - Must call the scheduler (per YAK specifications) for preemption
    //  - YKNewTask() is (and must be) REENTRANT

    static int taskIDCount; 
    TCBPtr newTCBPtr;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Allocate a new TCB in YKTasks
    newTCBPtr = allocateTCB();
    
    // Initialize task struct data
    newTCBPtr->stackBottom = ((int *)taskStack) - 1; // 1 integer (2 bytes) above taskStack
    newTCBPtr->taskID = taskIDCount++;
    newTCBPtr->delayCount = 0;
    newTCBPtr->priority = taskPriority;
    newTCBPtr->next = NULL;
    newTCBPtr->prev = NULL;  

    // Set SP to make room on stack for registers
    newTCBPtr->sp = newTCBPtr->stackBottom - (CONTEXT_SIZE-1);

    // Put values on to stack--general purpose reg values are arbitrary
    ((int *)taskStack)[-1]= DEFAULT_FLAGS;    // FLAGS - HIGH stack address = BOTTOM
    ((int *)taskStack)[-2] = 0;               // CS 
    ((int *)taskStack)[-3] = (int) taskCode;  // IP
    ((int *)taskStack)[-4] = 1;               // AX
    ((int *)taskStack)[-5] = 2;               // BX
    ((int *)taskStack)[-6] = 3;               // CX
    ((int *)taskStack)[-7] = 4;               // DX
    ((int *)taskStack)[-8] = (int) taskStack; // BP 
    ((int *)taskStack)[-9] = 0;               // SI
    ((int *)taskStack)[-10] = 0;              // DI
    ((int *)taskStack)[-11] = 0;              // SS
    ((int *)taskStack)[-12] = 0;              // DS
    ((int *)taskStack)[-13] = 0;              // ES - LOW stack address = TOP

    // Insert task in ready queue
    insertReady(newTCBPtr);

    // If interrupts were enabled on entering function, reenable
    if (iStatus) 
    {
        YKExitMutex();
    }    

    // Call scheduler if kernel has been started
    if (YKStartedFlag)
    {        
        YKScheduler();
    }

    return;
}

void YKRun()
{
    // Start the RTOS executing task
    //  - Must be called EXACTLY once to start the system--cannot be called multiple times
    //  - The call to YKRun should never return since YAK will always have at least 1 task
    //  - Must call the scheduler (per YAK specifications)
    
    // Set start flag
    YKStartedFlag = 1;

    // Enable interrupts (YKInitialize disabled them)
    YKExitMutex();

    // Start system
    YKScheduler();

    printString("ERROR in YKRun(): function should not return!\n\r");

    return;
}

void YKScheduler()
{
    // Schedule highest priority task to run
    //  - Increment YKCtxSwCount iff currently running task is no longer highest priority
    //  - Note: Dispatcher() is only called in the case of a context switch

    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();
    
    if (YKReadyHead->taskID != YKCurrentTask->taskID)
    {
        YKCtxSwCount++;
        YKDispatcher();
    }

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return;
}

void YKEnterISR()
{
    // Keep track of interrupt nesting level
    //  - Should be called near the beginning of each ISR before reenabling interrupts

    YKInterruptNestLevel++;   
    
    return;
}

void YKExitISR()
{
    // Perform necessary clean up on exiting ISR
    //  - Must call the scheduler if returning to task code (per YAK specifications)
    //  - Should be called near end of ISR (after handlers) while interrupts are disabled

    // Decrement interrupt nesting global var
    YKInterruptNestLevel--;

    // Check for return to task code
    if (YKInterruptNestLevel == 0)
        YKScheduler();

    return;
}

void YKDelayTask(unsigned count)
{
    // Delay currently running task by count
    //  - Note: per YAK specifications, tasks can only delay themselves
    //  - Must call the scheduler (per YAK specifications)
    //  - This function does not change YKCurrentTask--this is left to the scheduler
    //  - Function should not be called with count less than or equal to 0!
    //  - YKDelayTask() is (and must be) REENTRANT
    
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Check for bad delay count
    if (count <= 0)
    {
        printString("ERROR in YKDelayTask(): invalid delay count.\n\r");
    }

    // Remove currently running task from YKReadyList
    removeReady(YKCurrentTask);
    
    // Set task delay
    YKCurrentTask->delayCount = count;

    // Add to delayed list
    insertDelayed(YKCurrentTask);

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    YKScheduler();

    return;
}

YKSEM* YKSemCreate(int initialValue)
{
    // Create and initialize a semaphore
    //  - Must be called EXACTLY once per semaphore
    //  - Should typically be called in main before starting kernel
    //  - The initial value must be non-negative

    YKSEM *newSem;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Allocate new semaphore
    newSem = allocateSemaphore();

    // Initialize data
    newSem->value = initialValue;
    newSem->pendHead = NULL;

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return newSem;
}

void YKSemPend(YKSEM *semPtr)
{
    // Obtain a semaphore
    //  - Test the value of semaphore pointed to by semPtr and decrement it
    //  - Return to caller if initial semaphore value was greater than 0
    //  - Suspend and call scheduler if initial semaphore value was 0 or less
    //  - Note: ISRs and handlers can NEVER pend on a semaphore, only tasks!

    int oldVal;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Get original value and update new value
    oldVal = semPtr->value;
    semPtr->value -= 1;

    // Check if task should pend on semaphore
    if (oldVal <= 0)
    {
        // Remove currently running task from YKReadyList
        removeReady(YKCurrentTask);
    
        // Add to pending list
        insertPendSem(YKCurrentTask, semPtr);

        // Schedule next task
        YKScheduler();    
    }

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }
    
    return;

}

void YKSemPost(YKSEM *semPtr)
{
    // Release a semaphore
    //  - Increment the value of semaphore pointed to by semPtr
    //  - Ready the highest priority task pending on given semaphore
    //  - Call scheduler only if called from task code
    //  - Note: ISRs, handlers, and tasks may all post on a semaphore
    //  - Note: The semaphore is an integer rather than binary--posting to the semaphore in 
    //      rapid sucession will result in a large semaphore value

    int oldVal;
    TCBPtr tmp;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Get original value and update new value
    oldVal = semPtr->value;
    semPtr->value += 1;

    // Ready task if some task was pending on semaphore
    if (oldVal < 0) 
    {
        // Ready the highest priority task wait for this semaphore
        tmp = removePendSem(semPtr);
        insertReady(tmp);
        
        // Call scheduler (if not in ISR)
        if (YKInterruptNestLevel == 0)
        {
            YKScheduler();
        }
    }

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return;
}

YKQ* YKQCreate(void **base, unsigned size)
{
    // Create and initialize a message queue
    //  - Must be called EXACTLY once per queue
    //  - Typically called in main in user code
    //  - Parameter "base" specifies the base address of the queue, which is itself an 
    //      array of void pointers
    //  - Parameter "size" informs kernel of # of entries in queue (size of the array)

    YKQ *newQ;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Allocate space for queue management struct
    newQ = allocateQueue();

    // Initialize QStruct data
    newQ->numEntries = 0;
    newQ->maxEntries = size;
    newQ->start = base;
    newQ->end = base+(size-1);
    newQ->nextSlot = base;
    newQ->nextMsg = base;
    newQ->pendHead = NULL;

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return newQ;
}

void* YKQPend(YKQ *QPtr)
{
    // Obtain the oldest message from a queue
    //  - Return a void* to message if queue is non-empty
    //  - If queue is empty, caller is suspended until a message is available
    //  - Note: ISRs and handlers can NEVER pend on a queue, only tasks!    

    void *msg;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Check if task needs to wait for a message
    if (QPtr->numEntries == 0)
    {
        // Remove currently running task from YKReadyList
        removeReady(YKCurrentTask);
    
        // Add to pending list
        insertPendQ(YKCurrentTask, QPtr);

        // Schedule next task
        YKScheduler();    
    }

    // Return oldest message in queue
    msg = *(QPtr->nextMsg);

    // Advance nextMsg, wrapping if necessary
    if (QPtr->nextMsg == QPtr->end)
    {
        QPtr->nextMsg = QPtr->start;
    }
    else
    {
        (QPtr->nextMsg)++; 
    }

    // Update number of entries
    (QPtr->numEntries)--;

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return msg;
}

int YKQPost(YKQ *QPtr, void *msg)
{
    // Post a message to a queue
    //  - Returns 1 if message was successfully placed in queue
    //  - Returns 0 if queue was full (indicates that message was NOT placed in queue)
    //  - Highest priority task waiting for a message from this queue is made ready
    //  - Call scheduler only if called from task code
    //  - Note: ISRs, handlers, and tasks may all post to a queue

    TCBPtr tmp;
    int iStatus;

    // Store interrupt status and disable
    iStatus = YKEnterMutex();

    // Check if queue is full
    if (QPtr->numEntries >= QPtr->maxEntries)
    {
        // If interrupts were enabled on entering function, reenable
        if (iStatus)
        {
            YKExitMutex();
        }

        return 0;
    }

    // Place message in queue
    *(QPtr->nextSlot) = msg;

    // Advance nextSlot, wrapping if necessary
    if (QPtr->nextSlot == QPtr->end)
    {
        QPtr->nextSlot = QPtr->start;
    }
    else
    {
        (QPtr->nextSlot)++; 
    }

    // Update number of entries
    (QPtr->numEntries)++;

    // Ready any tasks waiting on a message in this queue
    if (QPtr->pendHead != NULL)
    {
        // Ready the highest priority task wait for this semaphore
        tmp = removePendQ(QPtr);
        insertReady(tmp);
        
        // Call scheduler (if not in ISR)
        if (YKInterruptNestLevel == 0)
        {
            YKScheduler();
        }
    }

    // If interrupts were enabled on entering function, reenable
    if (iStatus)
    {
        YKExitMutex();
    }

    return 1;
}
