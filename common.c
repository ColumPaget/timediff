#include "common.h"

double CalcTimediff(struct timeval *Prev, struct timeval *Curr)
{
    double secs=0, usec=0;

    if (Prev->tv_sec > 0)
    {
        //calcluate secs as a floating point value with millisecond resolution
        secs=difftime(Curr->tv_sec, Prev->tv_sec);
        if (Curr->tv_usec < Prev->tv_usec)
        {
            secs-=1; //we have rolled over our usec, so there's actually one fewer second
            usec=1000000 - Prev->tv_usec + Curr->tv_usec;
        }
        else usec=Curr->tv_usec - Prev->tv_usec;

        secs += usec / 1000000;
    }

    return(secs);
}


