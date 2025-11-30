#ifndef LPF_H_
#define LPF_H_

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

typedef struct
{
    double alpha;
    double y;
}
lpf_t;

extern void LPF_Init(lpf_t * filter);
extern double LPF_NextSample(lpf_t * filter, double x);

#endif /* LPF_H */

