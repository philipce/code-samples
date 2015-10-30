#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include "crc.c"

#define N_BUF 1000 // buffer size for nodes needing crc computation

// typedefs
typedef struct node_t
{
    char *name;
    unsigned int crc;
    char err;
    struct node_t *next;
    void *fb;
    size_t size;
} Node;

// globals
pthread_mutex_t mutex; // lock used by monitor code to protect globals
pthread_cond_t cv; // condition variable used to indicate node needing crc
volatile Node *node_buf[N_BUF]; // buffer of nodes needing crc computation
volatile int next_node; // index of next node needing a crc computation
volatile int next_post; // index of next spot to post node needing crc
volatile int terminate; // set to terminate threads

// prototypes
void *worker_func(void *arg);
volatile Node *get_node(void);
void post_node(Node *);
void insert(Node *, Node **, Node **);
void printNodes(Node *);
void freeNodes(Node *);


/**
 * Main function for multi-threaded directory checksum. Main thread walks the directories,
 * worker threads compute the checksums.
 *
 * Parameters: argv = [dir_name, n_threads]
 * - dir_name: name of directory to traverse
 * - n_threads: number of worker threads to create
 *
 * Returns: EXIT_SUCCESS if no errors occur otherwise EXIT_FAILURE
 *
 */
int main(int argc, char *argv[])
{
    char *dir_name;
    int n_threads;
    Node *head = NULL;
    Node *tail = NULL;  
    int status;

    // read in parameters
    if (argc != 3)
    {
        printf("Usage: problem_2 dir_name n_threads\n");
        return EXIT_FAILURE;
    }
    dir_name = argv[1];
    status = sscanf(argv[2], "%d", &n_threads);
    if (status != 1 || n_threads < 1 || n_threads > 99)
    {
        printf("Error: specify integer number of workers from 1-99\n");
        return EXIT_FAILURE;
    }         
        
    // initialize mutex and cv    
    status = pthread_mutex_init(&mutex, NULL);
    if (status != 0)
    {
        printf("Error: unable to initialize mutex (errno:%d)\n", status);
        return EXIT_FAILURE;
    }       
    status = pthread_cond_init(&cv, NULL);      
    if (status != 0)
    {
        printf("Error: unable to initialize cv (errno:%d)\n", status);
        return EXIT_FAILURE;
    }
        
    // create worker threads    
    pthread_t threads[n_threads];  
    int i;
    for (i = 0; i < n_threads; i++)
    {
        status = pthread_create(&threads[i], NULL, worker_func, NULL);        
        if (status != 0)
        {
            printf("Error: unable to create worker thread %d (errno:%d)\n", i, status);
            return EXIT_FAILURE;
        }    
    }   
    
    // open directory
    DIR *ds;
    ds = opendir(dir_name);
    if (ds == NULL)
    {
        printf("Error opening \'%s\', errno %d: %s\n", dir_name, errno, strerror(errno));
        return EXIT_FAILURE;
    }    
    
    // read files    
    #ifndef _DIRENT_HAVE_D_TYPE
        printf("Error: unable to distinguish entry types\n");
        return EXIT_FAILURE;
    #endif    
    struct dirent *de = readdir(ds);
    while(de != NULL)
    {    
        // check entry type -- skip directories
        if (de->d_type == DT_DIR)
        {
            de = readdir(ds); 
            continue;
        }

        // create node for entry             
        Node *n = malloc(sizeof(Node));
        if (n == NULL)
        {            
            printf("Error: unable to malloc Node\n");
            freeNodes(head);
            return EXIT_FAILURE;
        }
        
        // set file name and init node         
        n->name = de->d_name;
        n->next = NULL;
        n->crc = 0xDEADBEEF;
        
        // get stat struct for size of file
        struct stat s;
        char path[strlen(dir_name) + strlen(de->d_name) + 2];       
        strcpy(path, dir_name);
        strcat(path, "/");
        strcat(path,de->d_name);    
        int access_err = (stat(path, &s) != 0) ? 1 : 0;   
        n->size = s.st_size;               

        // read file into buffer       
        FILE *f = fopen(path, "rb");       
        access_err |= (f == NULL) ? 1 : 0;
        void *buf = malloc(n->size);
        if (buf == NULL)
        {
            printf("Error: unable to malloc file buffer\n");
            freeNodes(head);
            return EXIT_FAILURE;
        } 
        access_err |= (fread(buf, s.st_size, 1, f) != 1);        
        n->fb = buf;

        // flag access error
        if (access_err)
        {
            n->err = 1;
        }
        else
        {                                       
            n->err = 0;
        }
        
        // post node for crc computation        
        post_node(n);

        // clean up
        if (f != NULL && fclose(f) != 0)
        {
            printf("Error: fclose failed on \'%s\'\n", de->d_name);
            freeNodes(head);
            return EXIT_FAILURE;
        }               
        
        // place entry in list
        insert(n, &head, &tail);

        // read next entry
        de = readdir(ds);  
    }
    
    // signal all worker threads to terminate when buffer is empty
    status = pthread_mutex_lock(&mutex);
    if (status != 0)
    {
        printf("Error in get_node: unable to take mutex (errno:%d)\n", status);
    }
    terminate = 1;   
    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
    {
        printf("Error in get_node: unable to release mutex (errno:%d)\n", status);
    }
    
    // unblock any workers and wait for them to finish 
    post_node(NULL);
    for (i = 0; i < n_threads; i++)
    {
        status = pthread_join(threads[i], NULL);
        if (status != 0)
        {
            printf("Error: unable to join thread %d (errno:%d)\n", i, status);
            return EXIT_FAILURE;
        }                
    }
    
    // destroy mutex and cvs
    status = pthread_mutex_destroy(&mutex);
    if (status != 0)
    {
        printf("Error: unable to destroy mutex (errno:%d)\n", status);
        return EXIT_FAILURE;
    }
    status = pthread_cond_destroy(&cv);
    if (status != 0)
    {
        printf("Error: unable to destroy cv (errno:%d)\n", status);
        return EXIT_FAILURE;
    }
    
    // close directory
    if (closedir(ds) != 0)
    {
        printf("Error closingq \'%s\', errno %d: %s\n", dir_name, errno, strerror(errno));
        freeNodes(head);
        return EXIT_FAILURE;
    }

    // display results
    Node *cur = head;
    while (cur != NULL)
    {
        if (cur->err)
        {
            printf("%s ACCESS ERROR\n", cur->name);
        }   
        else
        {
            printf("%s %08X\n", cur->name, cur->crc);
        }
        cur = cur->next;
    }
    
    // clean up node list
    freeNodes(head);

    return EXIT_SUCCESS;
}


/**
 * Worker thread function
 */
void *worker_func(void *arg)
{
    while (1)
    {
        volatile Node *n = get_node();
        if (n == NULL) // null returned when threads are requested to terminate
        {
            break;
        }
        if (n->err)
        {
            n->crc = 0;
        }
        else
        {                                       
            n->crc = crc32(0, n->fb, n->size);
            free(n->fb);
            n->fb = NULL;
        }               
    }
    pthread_exit(NULL);
}
 
 
/**
 * Monitor functions. Allow thread-safe access to global variables.
 * 
 * get_node(void): return next node from global buffer
 * post_node(Node *): post given node to globabl buffer
 */
volatile Node *get_node(void)
{
    volatile Node *ret_node;

    // take lock
    int status = pthread_mutex_lock(&mutex);
    if (status != 0)
    {
        printf("Error in get_node: unable to take mutex (errno:%d)\n", status);
    }
    
    // wait for a node to be available    
    while (node_buf[next_node] == NULL) 
    {    
        if (terminate)
        {            
            ret_node = NULL;
            status = pthread_mutex_unlock(&mutex);
            if (status != 0)
            {
                printf("Error in get_node: unable to release mutex (errno:%d)\n", status);
            }
            return ret_node;            
        }    
        if (next_node >= N_BUF)
        {
             printf("Error in get_node: out of bounds access to node_buf[%d]\n", next_node);
        }       
        status = pthread_cond_wait(&cv, &mutex);
        if (status != 0)
        {
            printf("Error in get_node: failure waiting on cv (errno:%d)\n", status);
        }
    }

    // take node
    ret_node = node_buf[next_node];    
    node_buf[next_node] = NULL;
    next_node = (next_node + 1) % N_BUF;
    if (ret_node == NULL)
    {
        printf("Error in get_node: next node pointer is NULL\n");
    }

    // release lock
    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
    {
        printf("Error in get_node: unable to release mutex (errno:%d)\n", status);
    }
    
    return ret_node;
}

void post_node(Node *n)
{
    // take lock
    int status;
    status = pthread_mutex_lock(&mutex);
    if (status != 0)
    {
        printf("Error in post_node: unable to take mutex (errno:%d)\n", status);
    }

    // post node for worker threads to take
    if (next_post >= N_BUF)
    {
         printf("Error in post_node: out of bounds access to node_buf[%d]\n", next_post);
    }
    if (node_buf[next_post] != NULL)
    {
        printf("Error in post_node: node buffer full!\n");
    }   
    node_buf[next_post] = n;
    next_post = (next_post + 1) % N_BUF;
    status = pthread_cond_broadcast(&cv);
    if (status != 0)
    {
        printf("Error in post_node: unable to signal cv (errno:%d)\n", status);
    }
    
    // release lock
    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
    {
        printf("Error in post_node: unable to release mutex (errno:%d)\n", status);
    }
} 
 

/**
 * Insert node into linked list, sorted according to strcmp()
 *
 * Parameters: 
 * - Node *n - pointer to node to insert into list
 * - Node **h - pointer to head pointer of list to insert n into
 * - Node **t - pointer to tail pointer of list to insert n into 
 */
void insert(Node *n, Node **h, Node **t)
{
    // handle empty list
    if (*h == NULL || *t == NULL)
    {
        assert(*h == NULL && *t == NULL);
        *h = n;
        *t = n;
    }
    // handle list already containing elements
    else
    {      
        // handle case where n should be new head
        Node *cur = *h;
        int val = strcmp(n->name, cur->name);
        if (val < 0)
        {
            *h = n;
            n->next = cur;
        }
        
        // otherwise, find n a place in the remaining list
        else
        {            
            Node *prev = *h;            
            cur = (*h)->next;
            int placed = 0;                
            while (cur != NULL)
            {                            
                // check if new node should come before current
                val = strcmp(n->name, cur->name);
                if (val < 0)
                {
                    prev->next = n;
                    n->next = cur;
                    placed = 1;
                    break;
                } 
                
                prev = cur;
                cur = cur->next;                
            }
            // if not yet placed, node should be new tail      
            if (!placed)
            {
                (*t)->next = n;
                *t = n;
            }    
        }
    }   
}


/**
 * Print out Node list
 *
 * Parameters: 
 *      - Node *h - pointer to the head of the list to print
 */
void printNodes(Node *h)
{
    printf("  * Node list contents:\n");
    Node *cur = h;
    while (cur != NULL)
    {
        printf("\t%s --> \n", cur->name);
        cur = cur->next;
    }    
}


/**
 * Free Node list
 *
 * Parameters: 
 *      - Node *h - pointer to the head of the list to free
 */
void freeNodes(Node *h)
{
    Node *cur = h;
    while (cur != NULL)
    {    
        Node *temp = cur;
        cur = cur->next;
        free(temp);
    }
}





