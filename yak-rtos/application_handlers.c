//------------------------------------------------------------------------
// Sample application code - demo Message queues
//  - Description:  Sample interrupt handler code
//------------------------------------------------------------------------

#include "application.h"
#include "yakk.h"
#include "clib.h"

extern YKQ *MsgQPtr; 
extern struct msg MsgArray[];
extern int GlobalFlag;

void myreset(void)
{
    exit(0);
}

void mytick(void)
{
    static int next = 0;
    static int data = 0;

    // create a message with tick (sequence #) and pseudo-random data
    MsgArray[next].tick = YKTickNum;
    data = (data + 89) % 100;
    MsgArray[next].data = data;
    if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
	printString("  TickISR: queue overflow! \n");
    else if (++next >= MSGARRAYSIZE)
	next = 0;
}	       

void mykeybrd(void)
{
    GlobalFlag = 1;
}
