#include "non_block_queue.h"



int non_block_queue_init(struct non_block_queue_struct * que_struct, int32_t queue_width, uint32_t queue_height, void * queue_buffer, uint32_t queue_buffer_size)
{
    // Sanity check for queue_buffer size, height and width
    // Check that queue_width and hight evenly 
    if((queue_width * queue_height) != queue_buffer_size)
    {
        return 1;
    }
   
    que_struct->backNdx = 0;
    que_struct->frontNdx = 0;
    que_struct->queue_buffer = queue_buffer;
    que_struct->queue_height = queue_height;
    que_struct->queue_width = queue_width;
    
    return 0;
}


sig_atomic_t non_block_queue_empty(struct non_block_queue_struct * que)
{
    return que->backNdx == que->frontNdx;
}    
    

void non_block_queue_push_front(struct non_block_queue_struct * que, void * data)
{
}


void non_block_queue_pop_back(struct non_block_queue_struct * que, void * data)
{
}


#if 0

bool empty()
{
    return frontNdx == backNdx;
}
    

// Read front and pop from queue
bool popFront(QType& retVal)
{
    // Exit if queue empty
    if(frontNdx == backNdx)
    {
        return false;
    }

    // Retrieve value
    retVal = queue[frontNdx];

    // Increment with circular wrap
    frontNdx = (frontNdx + 1) % QSize;

    return true;
}


// Write value and increment back pointer
bool pushBack(const QType& val)
{
    // Bump pointer and check for collision with front
    int nextBackNdx = (backNdx + 1) % QSize;

    // Check for collision with front
    // Queue is full, return no write
    if(nextBackNdx == frontNdx)
    {
        return false;
    }

    // Write value
    queue[backNdx] = val;

    // Advance pointer
    backNdx = nextBackNdx;

    return true;
}

#endif