/* cacheQueue - a cache of synchronized message queues.
 * ParaFlow uses these to implement the thread synchronization
 * when parallel processing. */


#include "common.h"
#include "synQueue.h"
#include "cacheQueue.h"

/* This is a synQueue that contains synQueues */
static struct synQueue *queueCache;

struct synQueue *cacheQueueAlloc()
/* Get a synQueue from cache */
{
uglyf("3.1.3.0\n");
struct synQueue *queue;
uglyf("3.1.3.1\n");
if (queueCache == NULL)
    queueCache = synQueueNew();
uglyf("3.1.3.2\n");
if ((queue = synQueueGrab(queueCache)) == NULL)
    queue = synQueueNew();
uglyf("3.1.3.3\n");
return queue;
}
    
void cacheQueueFree(struct synQueue **pQueue)
/* Free up queue back to hash. The queue must be empty. */
{
struct synQueue *queue = *pQueue;
if (queue)
    {
    assert(synQueueSize(queue) == 0);
    synQueuePut(queueCache, queue);
    }
*pQueue = NULL;
}

