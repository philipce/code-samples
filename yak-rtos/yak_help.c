//------------------------------------------------------------------------
// YAK Helper Function Code
//  - Description: Helper functions used by the YAK kernel
//------------------------------------------------------------------------

#include "clib.h"
#include "kernel.h"
#include "helper.h"
#include "user.h"

//--------------------------------
// HELPER FUNCTIONS
//--------------------------------
void insertReady(TCBPtr newTCB)
{
    // Add TCB to ready task queue
    //  - Ready task priority queue is implemented as a doubly linked list with the beginning 
    //      and end defined by YKReadyHead and YKReadyTail
    //  - Highest prority is 1 with high priority tasks stored at the list head
    //  - This function DOES NOT remove a delayed task's TCB from the delayed list--TCB MUST be
    //      removed from the delayed list before being inserted into the ready list
    //  - Note: YAK does not allow two tasks to have the same priority
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr curTCB;

    // Check for bad TCBPtr
    if (newTCB == NULL)
    {
        printString("ERROR in insertReady(): cannot insert NULL pointer.\n\r");
    }
    // Case 1: no tasks already in queue
    else if (YKReadyHead == NULL)
    {
        YKReadyHead = newTCB;
        YKReadyTail = newTCB;
        newTCB->prev = NULL;
        newTCB->next = NULL;
    }
    // Case 2: one or more tasks already in queue
    else
    {
        // Case 2.1 - new task is highest priority
        if (newTCB->priority < YKReadyHead->priority)
        {
            YKReadyHead->prev = newTCB;
            newTCB->next = YKReadyHead;
            newTCB->prev = NULL;
            YKReadyHead = newTCB;
        }
        // Case 2.2 - new task is lowest priority
        else if (newTCB->priority > YKReadyTail->priority)
        {
            // Note: this shouldn't happen since Idle should ALWAYS be ready and lowest priority
            printString("ERROR in insertReady(): New task priority shouldn't be lowest in ready queue!\n\r");
            YKReadyTail->next = newTCB;
            newTCB->prev = YKReadyTail;
            newTCB->next = NULL;
            YKReadyTail = newTCB;
        }
        // Case 2.3 - new task is in the middle
        else
        {
            // Find where to insert
            curTCB = YKReadyHead;
            while(curTCB->priority < newTCB->priority)
                curTCB = curTCB->next;

            // Place newTCB before curTCB
            curTCB->prev->next = newTCB;
            newTCB->prev = curTCB->prev;
            curTCB->prev = newTCB;
            newTCB->next = curTCB;
        }        
    }

    return;
}

void removeReady(TCBPtr remTask)
{
    // Remove task from the list of ready tasks
    //  - This function does not ensure that remTask is in the ready list - caller must 
    //      ensure that remTask is actually in the ready list
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly      

    // Enforce that idle task cannot be delayed
    if (remTask->taskID == IDLE_TASK_ID)
    {
        printString("ERROR in removeReady(): Cannot remove IDLE task from ready list!\n\r");
    }
    // Case 1: no tasks in ready list
    else if (YKReadyHead == NULL)
    {
        printString("ERROR in removeReady(): Trying to remove from an empty list.\n\r");
    }
    // Case 2: remTask is the head and tail (only task in the list)
    else if (YKReadyHead == remTask && YKReadyTail == remTask)
    {
        YKReadyHead = NULL;
        YKReadyTail = NULL;        
        remTask->prev = NULL;
        remTask->next = NULL;
    }
    // Case 3: remTask is the head but not tail
    else if (YKReadyHead == remTask)
    {
        YKReadyHead = remTask->next;
        remTask->next->prev = NULL;
        remTask->next = NULL;
    }
    // Case 4: remTask is the tail but not head
    else if (YKReadyTail == remTask)
    {
        YKReadyTail = remTask->prev;
        remTask->prev->next = NULL;
        remTask->prev = NULL;
    }
    // Case 5: remTask is neither head nor tail
    else
    {
        remTask->next->prev = remTask->prev;
        remTask->prev->next = remTask->next;
        remTask->next = NULL;
        remTask->prev = NULL;        
    }

    return;
}

void insertDelayed(TCBPtr newTCB)
{
    // Add TCB to delayed task list
    //  - Delayed task priority queue is implemented as doubly linked list with the beginning 
    //      defined by YKDelayedHead 
    //  - Priority is based on delayCount, with smallest delay being at the head
    //  - The delay of a particular task is its delay plus the sum of all previous tasks' delays
    //  - This function DOES NOT remove a ready task's TCB from the ready list--TCB MUST be 
    //      removed from the ready list before being inserted into the delayed list
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr curTCB;

    // Check for TCBPtr
    if (newTCB == NULL)
    {
        printString("ERROR in insertDelayed(): cannot insert NULL pointer.\n\r");
    }
    // Enforce that idle task cannot be delayed
    else if (newTCB->taskID == IDLE_TASK_ID)
    {
        printString("ERROR in insertDelayed(): Cannot delay IDLE task!\n\r");
    }    
    // Case 1: no tasks already in queue
    else if (YKDelayedHead == NULL)
    {
        YKDelayedHead = newTCB;
        YKDelayedTail = newTCB;
        newTCB->prev = NULL;
        newTCB->next = NULL;
    }
    // Case 2: one or more tasks already in queue
    else
    {
        // Case 2.1: new task is smallest delay
        if (newTCB->delayCount < YKDelayedHead->delayCount)
        {
            // Update next task's delay
            YKDelayedHead->delayCount -= newTCB->delayCount;

            // Place task
            YKDelayedHead->prev = newTCB;
            newTCB->next = YKDelayedHead;
            newTCB->prev = NULL;
            YKDelayedHead = newTCB;
        }
        // Case 2.3: new task is in the middle
        else
        {
            // Find where to place newTCB and update newTCB's delay
            curTCB = YKDelayedHead;
            while ((newTCB->delayCount >= curTCB->delayCount) && (curTCB != NULL))
            {
                newTCB->delayCount -= curTCB->delayCount;
                curTCB = curTCB->next;              
            }
            // Place task at the list end
            if (curTCB == NULL)
            {
                YKDelayedTail->next = newTCB;
                newTCB->prev = YKDelayedTail;
                newTCB->next = NULL;
                YKDelayedTail = newTCB;
            }
            // Place task in list middle, before curTCB
            else
            {
                curTCB->prev->next = newTCB;
                newTCB->prev = curTCB->prev;
                curTCB->prev = newTCB;
                newTCB->next = curTCB;
            
                // Update delay of task after newTCB
                newTCB->next->delayCount -= newTCB->delayCount;                
            }            
        }
    }

    return;
}

TCBPtr removeDelayed()
{
    // Find the first task in the delay list with zero remaining delay and remove it
    //  - Returns a pointer to the task removed if one is found
    //  - Returns NULL if there is no task in the delay list with a delay of zero
    //  - Note: more than one task may have a delay of zero! A call to this function will only
    //      take the first one in the list
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr tmp;

    // Default return value
    tmp = NULL;

    // Check if delayed list is empty
    if (YKDelayedHead == NULL)
    {
        return tmp;
    }
    // Check if some task is done being delayed
    if (YKDelayedHead->delayCount == 0)
    {
        tmp = YKDelayedHead;
        YKDelayedHead = YKDelayedHead->next;
        YKDelayedHead->prev = NULL;
        tmp->next = NULL;
    }

    return tmp;
}

void insertPendSem(TCBPtr newTCB, YKSEM *ptr)
{
    // Insert task into the list of tasks pending on specified semaphore
    //  - Tasks in pending list are ordered by task priority
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr curTCB;

    // Check for bad TCBPtr
    if (newTCB == NULL)
    {
        printString("ERROR in insertPendSem(): cannot insert NULL pointer.\n\r");
    }
    // Case 1: no tasks already pending
    else if (ptr->pendHead == NULL)
    {
        ptr->pendHead = newTCB;
        newTCB->prev = NULL;
        newTCB->next = NULL;
    }
    // Case 2: one or more tasks already pending
    else
    {
        // Case 2.1 - new task is highest priority
        if (newTCB->priority < ptr->pendHead->priority)
        {
            ptr->pendHead->prev = newTCB;
            newTCB->next = ptr->pendHead;
            newTCB->prev = NULL;
            ptr->pendHead = newTCB;
        }
        // Case 2.2 - new task is not highest priority
        else
        {
            // Find pending task with next higher priority (lower number) than new task
            curTCB = ptr->pendHead;
            while (curTCB->priority < newTCB->priority)
            {
                if (curTCB->next == NULL)
                {
                    break;
                }
                else
                {
                    curTCB = curTCB->next;
                }
            }

            // Special handling if curTCB is the last in the list
            if ((curTCB->next == NULL) && (newTCB->priority > curTCB->priority))
            {   
                curTCB->next = newTCB;
                newTCB->prev = curTCB;
                newTCB->next = NULL;
            }
            // Place task before curTCB otherwise
            else
            {
                curTCB->prev->next = newTCB;
                newTCB->prev = curTCB->prev;
                curTCB->prev = newTCB;
                newTCB->next = curTCB;
            }
        }
    }  

    return;
}

TCBPtr removePendSem(YKSEM *ptr)
{
    // Remove task from list of tasks pending on specified semaphore
    //  - Task with highest priority is removed first
    //  - Function returns NULL if no task is pending on this semaphore
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr tmp;

    // Check for empty pending list otherwise remove and return head of pending list
    if (ptr->pendHead == NULL)
    {
        tmp = NULL;
    }
    else
    {
        tmp = ptr->pendHead;
        ptr->pendHead = ptr->pendHead->next;
        ptr->pendHead->prev = NULL;
        tmp->next = NULL;
    }

    return tmp;
}

void insertPendQ(TCBPtr newTCB, YKQ *ptr)
{
    // Insert task into the list of tasks pending on specified queue
    //  - Tasks in pending list are ordered by task priority
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr curTCB;

    // Check for bad TCBPtr
    if (newTCB == NULL)
    {
        printString("ERROR in insertPendQ(): cannot insert NULL pointer.\n\r");
    }
    // Case 1: no tasks already pending
    else if (ptr->pendHead == NULL)
    {
        ptr->pendHead = newTCB;
        newTCB->prev = NULL;
        newTCB->next = NULL;
    }
    // Case 2: one or more tasks already pending
    else
    {
        // Case 2.1 - new task is highest priority
        if (newTCB->priority < ptr->pendHead->priority)
        {
            ptr->pendHead->prev = newTCB;
            newTCB->next = ptr->pendHead;
            newTCB->prev = NULL;
            ptr->pendHead = newTCB;
        }
        // Case 2.2 - new task is not highest priority
        else
        {
            // Find pending task with next higher priority (lower number) than new task
            curTCB = ptr->pendHead;
            while (curTCB->priority < newTCB->priority)
            {
                if (curTCB->next == NULL)
                {
                    break;
                }
                else
                {
                    curTCB = curTCB->next;
                }
            }

            // Special handling if curTCB is the last in the list
            if ((curTCB->next == NULL) && (newTCB->priority > curTCB->priority))
            {   
                curTCB->next = newTCB;
                newTCB->prev = curTCB;
                newTCB->next = NULL;
            }
            // Place task before curTCB otherwise
            else
            {
                curTCB->prev->next = newTCB;
                newTCB->prev = curTCB->prev;
                curTCB->prev = newTCB;
                newTCB->next = curTCB;
            }
        }
    }  

    return;
}

TCBPtr removePendQ(YKQ *ptr)
{
    // Remove task from list of tasks pending on specified queue
    //  - Task with highest priority is removed first
    //  - Function returns NULL if no task is pending on this queue
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    TCBPtr tmp;

    // Check for empty pending list otherwise remove and return head of pending list
    if (ptr->pendHead == NULL)
    {
        tmp = NULL;
    }
    else
    {
        tmp = ptr->pendHead;
        ptr->pendHead = ptr->pendHead->next;
        ptr->pendHead->prev = NULL;
        tmp->next = NULL;
    }

    return tmp;
}

TCBPtr allocateTCB()
{
    // Allocate memory for a TCB
    //  - Returns a pointer to a free location in memory where a TCB can be stored
    //  - All memory allocated to TCBs is contained in the global array YKTasks
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    static int numTasksCreated;

    // Check space available
    if (numTasksCreated >= MAX_TASKS+1) // +1 is for idle task
    {
        printString("ERROR in allocateTCB(): storage space exceeded.\n\r");    
    }

    // Return the next available TCB
    return &(YKTasks[numTasksCreated++]);
}

YKSEM* allocateSemaphore()
{
    // Alocate memory for a semaphore
    //  - Returns a pointer to a free location in memory where a semaphore can be stored
    //  - All memory allocated to semaphores is contained in the global array YKSemaphores
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    static int numSemaphoresCreated;    

    // Check space available
    if (numSemaphoresCreated >= MAX_SEMAPHORES)
    {
        printString("ERROR in allocateSemaphore(): storage space exceeded.\n\r");
    }

    // Return the next available semaphore
    return &(YKSemaphores[numSemaphoresCreated++]);
}

YKQ* allocateQueue()
{
    // Alocate memory for a queue
    //  - Returns a pointer to a free location in memory where a queue can be stored
    //  - All memory allocated to queues is contained in the global array YKQueues
    //  - This function DOES NOT protect shared data--calls to this function MUST be enclosed 
    //      in a mutex to work properly

    static int numQueuesCreated;  

    // Check space available
    if (numQueuesCreated >= MAX_QUEUES)
    {
        printString("ERROR in allocateQueue(): storage space exceeded.\n\r");
    }

    // Return the next available queue
    return &(YKQueues[numQueuesCreated++]);
}
