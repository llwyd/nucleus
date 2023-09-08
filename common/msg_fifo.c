#include "msg_fifo.h"
#include <string.h>
#include <assert.h>

static critical_section_t * critical;

static void Enqueue( fifo_base_t * const base );
static void Dequeue( fifo_base_t * const base );
static void Flush( fifo_base_t * const base );
static void Peek( fifo_base_t * const base );

char dequeued_data[MSG_SIZE];

/* Need critical section... */

extern void Message_Init(msg_fifo_t * fifo, critical_section_t * crit)
{
    printf("Initialising Message FIFO\n");
    
    static const fifo_vfunc_t vfunc =
    {
        .enq = Enqueue,
        .deq = Dequeue,
        .flush = Flush,
        .peek = Peek,
    };
    FIFO_Init( (fifo_base_t *)fifo, MSG_FIFO_LEN );
    
    fifo->base.vfunc = &vfunc;
    critical = crit;
}

static void Enqueue( fifo_base_t * const base )
{
    assert( base != NULL );
    critical_section_enter_blocking(critical);
    
    msg_fifo_t * fifo = (msg_fifo_t *)base;

//    printf("ENQ read:%d, write:%d, Fill:%d\n", fifo->base.read_index, fifo->base.write_index, fifo->base.fill);
    /* Clear what was in there */
    memset(fifo->queue[ fifo->base.write_index ], 0x00, MSG_SIZE);

    int bytes_to_copy = fifo->data.len;
    assert(bytes_to_copy <= MSG_SIZE);
    /* Copy the new string */
    memcpy(fifo->queue[ fifo->base.write_index ], fifo->data.data, bytes_to_copy);

    fifo->base.write_index++;
    fifo->base.fill++;
    fifo->base.write_index = ( fifo->base.write_index & ( fifo->base.max - 1U ) );
//    printf("ENQ read:%d, write:%d, Fill:%d\n", fifo->base.read_index, fifo->base.write_index, fifo->base.fill);
    
    critical_section_exit(critical);
}

static void Dequeue( fifo_base_t * const base )
{
    critical_section_enter_blocking(critical);
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
//    printf("DEQ read:%d, write:%d, Fill:%d\n", fifo->base.read_index, fifo->base.write_index, fifo->base.fill);
    fifo->data.data = fifo->queue[ fifo->base.read_index ];
    memcpy(dequeued_data, fifo->data.data, MSG_SIZE);
    fifo->base.read_index++;
    fifo->base.fill--;
    fifo->base.read_index = ( fifo->base.read_index & ( fifo->base.max - 1U ) );
//    printf("DEQ read:%d, write:%d, Fill:%d\n", fifo->base.read_index, fifo->base.write_index, fifo->base.fill);
    
    critical_section_exit(critical);
}

extern char * Message_Get(void)
{
    return dequeued_data;
}

static void Peek( fifo_base_t * const base )
{
    critical_section_enter_blocking(critical);
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
//    printf("PK read:%d, write:%d, Fill:%d\n", fifo->base.read_index, fifo->base.write_index, fifo->base.fill);
    char * ptr = fifo->queue[ fifo->base.read_index ];
    
    fifo->data.data = ptr;
    critical_section_exit(critical);
}

static void Flush( fifo_base_t * const base )
{
    critical_section_enter_blocking(critical);
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->base.read_index = fifo->base.write_index;
    fifo->base.fill = 0U;
    critical_section_exit(critical);
}

