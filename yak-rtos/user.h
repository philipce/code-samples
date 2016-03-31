//------------------------------------------------------------------------
// YAK User Header File
//  - Description: User defined data used by YAK kernel
//------------------------------------------------------------------------

// User Constants
#define MAX_TASKS 64
#define MAX_SEMAPHORES 64
#define MAX_QUEUES 64
#define LOWEST_TASK_PRIORITY 100

// Global Data (defined in yak_c.c)
extern int YKCtxSwCount; // number of context switches
extern int YKIdleCount;  // used by idle task
extern int YKTickNum;    // incremented by tick handler
