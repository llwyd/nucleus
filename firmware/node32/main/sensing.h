#ifndef _SENSING_H_
#define _SENSING_H_


void Sensing_Init( void );
void Sensing_Read( void );
extern void Sensing_Task( void * pvParameters );
const float * Sensing_GetTemperature( void );

#endif /* _SENSING_H_ */
