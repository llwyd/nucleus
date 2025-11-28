#include "lpf.h"

extern void LPF_Init(lpf_t * filter)
{
    assert(filter != NULL);
    assert(filter->alpha > 0.0);
    assert(filter->alpha < 1.0);
    filter->y = 0.0;
}

extern double LPF_NextSample(lpf_t * filter, double x)
{
    assert(filter != NULL);
    filter->y = x - (filter->alpha * (x - filter->y));

    return filter->y;
}
