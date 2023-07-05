#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    char * name;
    bool (*event_fn)(void);
}
event_callback_t;

extern void Events_Init(void);

#endif /* EVENTS_H_ */
