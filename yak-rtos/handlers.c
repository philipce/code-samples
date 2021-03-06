//------------------------------------------------------------------------
// Interrupt Handlers
//  - Description: C code interrupt handlers called by ISRs
//------------------------------------------------------------------------

#include "clib.h"
#include "kernel.h"
#include "helper.h"
#include "application.h"

//--------------------------------
// GLOBAL VARIABLES
//--------------------------------
extern int KeyBuffer;           // defined in clib.s
extern TCBPtr YKDelayedHead;    // defined in yak_c.c
extern YKQ *MsgQPtr;            // defined in application.c
extern struct msg MsgArray[];   // defined in application.c
extern int GlobalFlag;          // defined in application.c

//--------------------------------
// HANDLER FUNCTIONS
//--------------------------------
void YKTickHandler(void)
{
    // Handle interrupts generated by system tick
    //  - Increment YKTickNum
    //  - Decrement delay on the top TCB in the delayed list
    //  - Ready tasks with an expired delay count
    //  - Post a message to message queue pointed to by MsgQPtr
    
    static int index;
    static int pseudoRand;
    TCBPtr tmp;

    YKEnterMutex();

    // Increment count
    YKTickNum++;

    // Handle delayed tasks
    if (YKDelayedHead != NULL)
    {
        // Decrement delay          
        YKDelayedHead->delayCount -= 1;

        // Check for tasks (possibly more than 1) to un-delay
        tmp = removeDelayed();
        while (tmp != NULL)
        {
            insertReady(tmp);
            tmp = removeDelayed();
        }
    }

    // Create a message with sequence # and pseudo-random data
    MsgArray[index].tick = YKTickNum;
    pseudoRand = (pseudoRand + 89) % 100;
    MsgArray[index].data = pseudoRand;

    // Post message to queue and check for queue overflow
    if (YKQPost(MsgQPtr, (void *) &(MsgArray[index])) == 0)
    {
        printString("ERROR in YKTickHandler(): queue overflow!\n\r"); 
    }

    // Increment buffer index and reset if necessary
    index++;
    if (index >= MSGARRAYSIZE)
    {
        index = 0;
    }
    
    YKExitMutex();

    return;
}

void YKResetHandler()
{
    // Handle reset interrupt
    //  - Reset interrupt generated by <CTRL-R>

    exit(0);
}

void YKKeyHandler()
{
    // Handle key press interrupt
    //  - set GlobalFlag

    YKEnterMutex();
    
    GlobalFlag = 1;

    YKExitMutex();

    return;
}
