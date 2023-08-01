#include "msg_fifo.h"
#include <string.h>

static void Enqueue( fifo_base_t * const base );
static void Dequeue( fifo_base_t * const base );
static void Flush( fifo_base_t * const base );


static void Enqueue( fifo_base_t * const base )
{
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;

    /* Clear what was in there */
    memset(fifo->queue[ fifo->base.write_index ], 0x00, MSG_SIZE);

    /* Copy the new string */
    strncpy(fifo->queue[ fifo->base.write_index ], fifo->data, MSG_SIZE);

    fifo->base.write_index++;
    fifo->base.fill++;
    fifo->base.write_index = ( fifo->base.write_index & ( fifo->base.max - 1U ) );
}

static void Dequeue( fifo_base_t * const base )
{
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->data = fifo->queue[ fifo->base.read_index ];
    fifo->base.read_index++;
    fifo->base.fill--;
    fifo->base.read_index = ( fifo->base.read_index & ( fifo->base.max - 1U ) );
}

static void Flush( fifo_base_t * const base )
{
    assert( base != NULL );
    msg_fifo_t * fifo = (msg_fifo_t *)base;
    fifo->base.read_index = fifo->base.write_index;
    fifo->base.fill = 0U;
}

