#include "msg_fifo.h"
#include <string.h>

static critical_section_t * critical;

static void Enqueue( fifo_base_t * const base );
static void Dequeue( fifo_base_t * const base );
static void Flush( fifo_base_t * const base );
static void Peek( fifo_base_t * const base );

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
//    critical_section_enter_blocking(critical);
//    cyw43_arch_lwip_begin();
    
    msg_fifo_t * fifo = (msg_fifo_t *)base;

    /* Clear what was in there */
    memset(fifo->queue[ fifo->base.write_index ], 0x00, MSG_SIZE);

    int bytes_to_copy = fifo->data.len;

    /* Copy the new string */
    memcpy(fifo->queue[ fifo->base.write_index ], fifo->data.data, bytes_to_copy);

    fifo->base.write_index++;
    fifo->base.fill++;
    fifo->base.write_index = ( fifo->base.write_index & ( fifo->base.max - 1U ) );
    
//    cyw43_arch_lwip_end();
//    critical_section_exit(critical);
}

static void Dequeue( fifo_base_t * const base )
{
//    critical_section_enter_blocking(critical);
//    cyw43_arch_lwip_begin();
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->data.data = fifo->queue[ fifo->base.read_index ];
    fifo->base.read_index++;
    fifo->base.fill--;
    fifo->base.read_index = ( fifo->base.read_index & ( fifo->base.max - 1U ) );
    
//    cyw43_arch_lwip_end();
//    critical_section_exit(critical);
}

static void Peek( fifo_base_t * const base )
{
//    critical_section_enter_blocking(critical);
//    cyw43_arch_lwip_begin();
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->data.data = fifo->queue[ fifo->base.read_index ];
    
//    cyw43_arch_lwip_end();
//    critical_section_exit(critical);
}

static void Flush( fifo_base_t * const base )
{
    critical_section_enter_blocking(critical);
    cyw43_arch_lwip_begin();
    
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->base.read_index = fifo->base.write_index;
    fifo->base.fill = 0U;
    cyw43_arch_lwip_end();
    critical_section_exit(critical);
}

