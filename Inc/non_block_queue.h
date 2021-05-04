#ifndef __non_block_queue_H
#define __non_block_queue_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <signal.h>
#include <stdint.h>     

struct non_block_queue_struct
{
    uint32_t queue_width;
    uint32_t queue_height;
    uint32_t _Atomic frontNdx;
    uint32_t _Atomic backNdx;
    void * queue_buffer;
};
     
// 
int non_block_queue_init(struct non_block_queue_struct * que, int32_t queue_width, uint32_t queue_height, void * queue_buffer, uint32_t queue_buffer_size);

sig_atomic_t non_block_queue_empty(struct non_block_queue_struct * que);

void non_block_queue_push_front(struct non_block_queue_struct * que, void * data);

void non_block_queue_pop_back(struct non_block_queue_struct * que, void * data);

         
#ifdef __cplusplus
}
#endif
#endif /*__non_block_queue_H */
